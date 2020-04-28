#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

//Includes de Sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "client.h"
#include "network_util.h"

void client_create(client_t* client,char* host,char* port){
    client->host = host;
    client->port = port;
    client->skt = 0;
    client->current_msg_id = 0;
}

int client_connect(client_t* client){
    struct addrinfo* results;

    if(!get_info_from_dns(client->host,client->port, &results, false)){
        fprintf(stderr,"No se pudo obtener la informacion del servidor DNS.\n");
        return -1;
    }

    if(!connect_to_available_server(&client->skt,results)){
        fprintf(stderr,"No se pudo conectar al servidor.\n");
        free(results);
        return -1;
    }
    
    free(results);
    return 1;
}

int client_send_msg(client_t* client,unsigned char* message,size_t msg_size){
    return send_message(client->skt,message,msg_size);
}

int client_recv_msg(client_t* client,unsigned char* buffer,size_t recieve_size){
    return recieve_message(client->skt,buffer,recieve_size);
}

void client_disconnect(client_t* client){
    shutdown(client->skt,SHUT_RDWR);
    close(client->skt);
}

void client_destroy(client_t* client){
    ;
}