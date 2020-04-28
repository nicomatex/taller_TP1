#define _POSIX_C_SOURCE 200809L

//Includes generales
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

//Includes de modulos

#include "server.h"
#include "command_parser.h"
#include "command_serializator.h"

#define ARG_PORT 1
#define MIN_ARGS 2
#define MAX_ARGS 2

#define HOST "localhost"

#define HEADER_BASE_SIZE 16
#define BUFFER_BASE_SIZE 32

#define POS_ARRAY_SIZE 12
#define POS_BODY_SIZE 4

#define RESPONSE "OK\n"
#define RESPONSE_SIZE 3

#define PARAM_SEPARATOR ","

uint32_t decode_int(unsigned char* message,size_t position){
    uint32_t integer;
    memcpy(&integer,&message[position],sizeof(uint32_t));
    return integer;
}

void print_command(command_t* command){
    printf("* Id: 0x%.4x\n",command->msg_id);
    printf("* Destino: %s\n",command->destination);
    printf("* Path: %s\n",command->path);
    printf("* Interface: %s\n",command->interface);
    printf("* Método: %s\n",command->method);

    if(command->signature_param_count > 0){
        printf("* Parámetros:\n");
        char* parameter;
        parameter = strtok(command->signature_parameters,PARAM_SEPARATOR);

        while(parameter != NULL){
            printf("    * %s\n",parameter);
            parameter = strtok(NULL,PARAM_SEPARATOR);
        }
    }
    printf("\n");
}

void run(server_t* server){

    unsigned char* buffer = malloc(BUFFER_BASE_SIZE*sizeof(char));
    size_t buffer_max_size = BUFFER_BASE_SIZE;
    size_t bytes_recieved = server_recv_msg(server,&buffer[0],HEADER_BASE_SIZE);
    size_t buffer_size = HEADER_BASE_SIZE;

    while(bytes_recieved > 0){

        size_t param_array_size = decode_int(buffer,POS_ARRAY_SIZE);
        size_t body_size = decode_int(buffer,POS_BODY_SIZE);

        if(buffer_size + param_array_size + body_size > buffer_max_size){
            buffer = realloc(buffer,HEADER_BASE_SIZE + param_array_size + body_size);
            buffer_max_size = HEADER_BASE_SIZE + param_array_size + body_size;
        }

        server_recv_msg(server,&buffer[HEADER_BASE_SIZE],param_array_size + body_size);
        char* response = RESPONSE;
        server_send_msg(server,(unsigned char*)response,RESPONSE_SIZE);

        command_t command_recieved;
        command_create(&command_recieved);
        decode_dbus_message(&buffer[0],&command_recieved);
        print_command(&command_recieved);
        command_destroy(&command_recieved);

        buffer_size = 0;
        bytes_recieved = server_recv_msg(server,&buffer[0],HEADER_BASE_SIZE);
    }

    free(buffer);
}

/*Chequeo de cantidad de parametros*/
bool check_parameters(int argc){ 
    if(argc < MIN_ARGS){
        fprintf(stderr,"Parametros insuficientes.");
        return false;
    }else if(argc > MAX_ARGS){
        fprintf(stderr,"Demasiados parametros.");
        return false;
    }
    return true;
}

int main(int argc, char *argv[]){
    if(!check_parameters(argc)) return -1;

    server_t server;

    server_create(&server,argv[ARG_PORT]);

    if(server_connect(&server) < 0){
        server_destroy(&server);
    }

    if(server_accept_conection(&server) < 0){
        server_destroy(&server);
    }

    run(&server);

    server_disconnect(&server);
    server_destroy(&server);

    return 0;
}