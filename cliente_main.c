#define _POSIX_C_SOURCE 200809L

#define ARG_HOST 1 
#define ARG_PORT 2
#define ARG_FILE 3

#define CHAR_NEWLINE '\n'

//Includes generales
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>

//Includes de Sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

//Includes de modulos del programa
#include "client.h"
#include "file_streamer.h"
#include "command_parser.h"
#include "command_serializator.h"

#define MIN_ARGS 3
#define MAX_ARGS 4

bool check_parameters(int argc){ /*Chequeo de cantidad de parametros*/
    if(argc < MIN_ARGS){
        fprintf(stderr,"Parametros insuficientes.");
        return false;
    }else if(argc > MAX_ARGS){
        fprintf(stderr,"Demasiados parametros.");
        return false;
    }
    return true;
}

void parse_and_send(char* line,void* client){
    client = (client_t*)client;
    command_t command;
    command_create(&command);

    if(!command_parse(line,&command)){
        fprintf(stderr,"Comando no valido\n");
        return;
    }

    size_t msg_size;
    unsigned char* message = generate_dbus_message(&command,8,&msg_size);\
    printf("DEBUG: message size: %ld\n",msg_size);

    size_t bytes_sent = client_send_msg(client,message,msg_size);
    printf("DEBUG: bytes sent: %ld\n",bytes_sent);

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
