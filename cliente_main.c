#define _POSIX_C_SOURCE 200809L

#define ARG_HOST 1 
#define ARG_PORT 2

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
#include "network_util.h"

bool check_parameters(int argc){
    if (argc != 3 || argc != 4){
        return false;
    }
    return true;
}

int main(int argc, char *argv[]){
    
    //if (!check_parameters(argc)) fprintf(stderr,"Error en la cantidad de parametros ingresada.\n");

    char* current_line = NULL;
    struct addrinfo* results;

    if(!get_info_from_dns(argv[ARG_HOST],argv[ARG_PORT], &results, false)){
        fprintf(stderr,"No se pudo obtener la informacion del servidor DNS.\n");
        return -1;
    }
    printf("Obtenida informacion DNS.\n");

    int skt;

    if(!connect_to_available_server(&skt,results)){
        fprintf(stderr,"No se pudo conectar al servidor.\n");
        free(results);
        return -1;
    }

    free(results);
    printf("Conectado al servidor.\n");

    char* mensaje_prueba = "FOR THE HORDE!!!\n";
    size_t bytes_sent = send_message(skt,mensaje_prueba,strlen(mensaje_prueba));

    if(bytes_sent == -1){
        fprintf(stderr,"Hubo un problema enviando el mensaje.\n");
    }else{
        printf("%ld bytes enviados con exito!\n",bytes_sent);
    }
    shutdown(skt,SHUT_RDWR);
    close(skt);
    
    char* buffer = NULL;
    size_t read_bytes = 0;

    return 0;
}
