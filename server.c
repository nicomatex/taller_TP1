#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "server.h"
#include "network_util.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

void server_create(server_t* server,char* port){
    server->port = port;
    server->skt = 0;
    server->client_skt = 0;
}

int server_connect(server_t* server){
    struct addrinfo* results;

    if ( !get_info_from_dns(NULL,server->port, &results, true) ){
        fprintf(stderr,"No se pudo obtener la informacion del servidor DNS.\n");
        return -1;
    }

    if ( !start_listening(&server->skt,results) ){
        fprintf(stderr,"No se pudo iniciar el servidor.\n");
        free(results);
        return -1;
    }

    free(results);
    return 1;
}

int server_accept_conection(server_t* server){
    server->client_skt = accept(server->skt,NULL, NULL);
    if ( server->client_skt == - 1 ){
        fprintf(stderr,"Error: %s.\n",strerror(errno));
        return -1;
    }
    return 1;
}

int server_send_msg(server_t* server,unsigned char* message,size_t msg_size){
    return send_message(server->client_skt,message,msg_size);
}

int server_recv_msg(server_t* server,unsigned char* buffer,size_t recieve_size){
    return recieve_message(server->client_skt,buffer,recieve_size);
}

void server_disconnect(server_t* server){
    shutdown(server->skt,SHUT_RDWR);
    close(server->skt);
}

void server_destroy(server_t* server){}
