#define _POSIX_C_SOURCE 200809L

#define HOST "localhost"

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

#include "network_util.h"

int main(int argc, char *argv[]){
    struct addrinfo* results;

    if(!get_info_from_dns(HOST,argv[ARG_PORT], &results, true)){
        fprintf(stderr,"No se pudo obtener la informacion del servidor DNS.\n");
        return -1;
    }

    return 0;
}