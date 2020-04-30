#define _POSIX_C_SOURCE 200809L

#define ARG_HOST 1 
#define ARG_PORT 2
#define ARG_FILE 3

#define MIN_ARGS 3
#define MAX_ARGS 4

#define RESPONSE_SIZE 3

#define CHAR_NEWLINE '\n'

//Includes generales
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

//Includes de modulos del programa
#include "client.h"
#include "file_streamer.h"
#include "command_parser.h"
#include "command_serializator.h"

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

void parse_and_send(char* line,void* context){
    client_t* client = (client_t*) context;
    command_t command;
    command_create(&command);

    if(!command_parse(line,&command)){
        fprintf(stderr,"Comando no valido\n");
        return;
    }

    command.msg_id = client->current_msg_id;

    size_t msg_size;
    
    unsigned char* message = generate_dbus_message(&command,&msg_size);

    size_t bytes_sent = client_send_msg(client,message,msg_size);

    if(bytes_sent == 0){
        free(message);
        command_destroy(&command);
        fprintf(stderr,"Conexion cerrada desde el servidor");
    }

    unsigned char response_buffer[RESPONSE_SIZE+1];
    size_t bytes_recieved = client_recv_msg(client,&response_buffer[0],RESPONSE_SIZE);

    if(bytes_recieved == 0){
        free(message);
        command_destroy(&command);
        fprintf(stderr,"Conexion cerrada desde el servidor");
    }

    response_buffer[RESPONSE_SIZE] = '\0';
    printf("0x%.4x: %s",client->current_msg_id,response_buffer);

    client->current_msg_id += 1;
    
    free(message);
    command_destroy(&command);
}

int main(int argc, char *argv[]){
    if(!check_parameters(argc)) return -1;
    FILE* file = stdin;
    if(argc == MAX_ARGS){
        file = fopen(argv[ARG_FILE],"r");
    }

    /*Establecimiento de conexion*/
    client_t client;
    client_create(&client,argv[ARG_HOST],argv[ARG_PORT]);

    if(client_connect(&client) < 0){
        client_destroy(&client);
        if(file != stdin) fclose(file);
        return -1;
    }

    file_streamer_t file_streamer;
    file_streamer_create(&file_streamer,file,parse_and_send,CHAR_NEWLINE);
    if(file_streamer_run(&file_streamer,&client) < 0){
        fprintf(stderr,"Error de lectura");
    }
    file_streamer_destroy(&file_streamer);

    client_disconnect(&client);
    client_destroy(&client);
    if(file != stdin) fclose(file);
    return 0;
}
