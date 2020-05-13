#define ARG_HOST 1 
#define ARG_PORT 2
#define ARG_FILE 3

#define MIN_ARGS 3
#define MAX_ARGS 4

#define CHAR_NEWLINE '\n'

//Includes generales
#include <stdio.h>

//Includes de modulos del programa
#include "client.h"
#include "client_dbus.h"
#include "file_streamer.h"

int main(int argc, char *argv[]){
    if ( argc < MIN_ARGS ){
        fprintf(stderr,"Parametros insuficientes.");
        return -1;
    }else if ( argc > MAX_ARGS ){
        fprintf(stderr,"Demasiados parametros.");
        return -1;
    }

    FILE* file = stdin;
    if ( argc == MAX_ARGS ){
        file = fopen(argv[ARG_FILE],"r");
    }

    /*Establecimiento de conexion*/
    client_t client;
    client_create(&client,argv[ARG_HOST],argv[ARG_PORT]);

    if ( client_connect(&client) < 0 ){
        client_destroy(&client);
        if ( file != stdin ) fclose(file);
        return -1;
    }

    file_streamer_t file_streamer;
    file_streamer_create(&file_streamer,file,dbus_parse_and_send,CHAR_NEWLINE);
    if ( file_streamer_run(&file_streamer,&client) < 0 ){
        fprintf(stderr,"Error de lectura");
    }
    file_streamer_destroy(&file_streamer);

    client_disconnect(&client);
    client_destroy(&client);
    if ( file != stdin ) fclose(file);
    return 0;
}
