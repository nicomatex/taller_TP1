//Includes de modulos
#include "server.h"
#include "server_dbus.h"

//Includes generales
#include <stdio.h>

#define ARG_PORT 1
#define MIN_ARGS 2
#define MAX_ARGS 2

int main(int argc, char *argv[]){
    if ( argc < MIN_ARGS ){
        fprintf(stderr,"Parametros insuficientes.");
        return -1;
    }else if ( argc > MAX_ARGS ){
        fprintf(stderr,"Demasiados parametros.");
        return -1;
    }

    server_t server;

    server_create(&server,argv[ARG_PORT]);

    if ( server_connect(&server) < 0 ){
        server_destroy(&server);
    }

    if ( server_accept_conection(&server) < 0 ){
        server_destroy(&server);
    }

    dbus_server_run(&server);

    server_disconnect(&server);
    server_destroy(&server);

    return 0;
}
