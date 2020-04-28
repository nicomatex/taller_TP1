#include "network_util.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CLIENT_QUEUE 1

bool get_info_from_dns(char* host, char* port, struct addrinfo** result, bool is_server){
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = is_server ? AI_PASSIVE : 0;

    if(getaddrinfo(host, port, &hints, result) != 0){
        fprintf(stderr,"Error obteniendo informacion del servidor DNS.\n");
        return false;
    }
    return true;
}

bool connect_to_available_server(int* skt, struct addrinfo* results){
    struct addrinfo* current;
    bool is_connected = false;
    int socket_state = 0;

    /*Itera sobre todos los resultados devueltos por el servidor DNS, y se queda con el
    primero al que se logra conectar*/
    for(current = results; current != NULL && is_connected == false;current = current->ai_next){
        *skt = socket(current->ai_family,current->ai_socktype,current->ai_protocol);
        
        if(*skt == -1){
            fprintf(stderr,"Error: %s\n.",strerror(errno));
        }
        else
        {
            socket_state = connect(*skt,current->ai_addr,current->ai_addrlen);
            if (socket_state == -1){
                printf("Error: %s\n.",strerror(errno));
                close(*skt);
            }
            is_connected = (socket_state != -1);
        }
    }
    return is_connected;
}

bool start_listening(int* skt,struct addrinfo* results){
    *skt = socket(results->ai_family,results->ai_socktype,results->ai_protocol);

    if (*skt == -1){
        fprintf(stderr,"Error: %s\n.",strerror(errno));
        return false;
    }
    
    int val = 1;

    //Establece SO_REUSEADDR para evitar tiempos de espera de liberacion del socket
    if(setsockopt(*skt, SOL_SOCKET, SO_REUSEADDR, &val,sizeof(val)) == -1){
        fprintf(stderr,"Error: %s\n.",strerror(errno));
        close(*skt);
        return false;
    }

    //Bindea el socket al puerto indicado
    if(bind(*skt,results->ai_addr,results->ai_addrlen) == -1){
        fprintf(stderr,"Error: %s.\n",strerror(errno));
        close(*skt);
        return false;
    }

    //Establece el socket como pasivo
    if(listen(*skt,MAX_CLIENT_QUEUE) == -1){
        fprintf(stderr,"Error: %s.\n",strerror(errno));
        close(*skt);
        return false;
    }

    return true;
}

int send_message(int skt,unsigned char *output_buffer, size_t size) {
    size_t sent = 0;
    size_t bytes_sent = 0;
    bool valid_socket = true;

    while (sent < size && valid_socket) {
        bytes_sent = send(skt, &output_buffer[sent], size-sent, MSG_NOSIGNAL);

        if (bytes_sent == 0) return 0;
        
        if (bytes_sent == -1) {
            valid_socket = false;
        } else {
            sent += bytes_sent;
        }
    }
    if (valid_socket) {
        return (int)sent;
    } else {
        fprintf(stderr,"Hubo un error enviando el mensaje");
        return -1;
    }
}

int recieve_message(int skt,unsigned char* buffer, size_t size){
    bool socket_error = false;
    bool remote_socket_closed = false;
    size_t bytes_recieved = 0;
    size_t just_recieved = 0;

    while(bytes_recieved < size && !socket_error && !remote_socket_closed){
        just_recieved = recv(skt,&buffer[bytes_recieved],size - bytes_recieved,0);

        if(just_recieved == -1){
            fprintf(stderr,"Error: %s\n",strerror(errno));
            return -1;
        } else if(just_recieved == 0){
            return 0;
        }
        bytes_recieved += just_recieved;
    }

    return (int)bytes_recieved;
}