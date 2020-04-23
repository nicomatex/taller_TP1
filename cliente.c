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
#include "command_parser.h"


bool check_parameters(int argc){
    if (argc != 3 || argc != 4){
        return false;
    }
    return true;
}

bool get_info_from_dns(char* host, char* port, struct addrinfo** result){
    struct addrinfo hints;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;

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

int send_message(int skt, char *output_buffer, size_t size) {
   size_t sent = 0;
   int bytes_sent = 0;
   bool valid_socket = true;

   while (sent < size && valid_socket) {
      bytes_sent = send(skt, &output_buffer[sent], size-sent, MSG_NOSIGNAL);

      if (bytes_sent == 0) {
         valid_socket = false;
      }
      else if (bytes_sent == -1) {
         valid_socket = false;
      }
      else {
         sent += bytes_sent;
      }
   }

   if (valid_socket) {
      return sent;
   }
   else {
      return -1;
   }
}

int main(int argc, char *argv[]){
    
    //if (!check_parameters(argc)) fprintf(stderr,"Error en la cantidad de parametros ingresada.\n");

    char* current_line = NULL;
    struct addrinfo* results;

    if(!get_info_from_dns(argv[ARG_HOST],argv[ARG_PORT], &results)){
        fprintf(stderr,"No se pudo obtener la informacion del servidor DNS.\n");
        return -1;
    }
    printf("Obtenida informacion DNS.\n");

    int skt;

    if(!connect_to_available_server(&skt,results)){
        fprintf(stderr,"No se pudo conectar al servidor.\n");
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
    
    /* 
    command_t command;
    while(getline(&current_line,&read_bytes,stdin) > 0){
        memset(&command,0, sizeof(command_t));
        if(!parse_command(&command,current_line)){
            printf("Error en un comando.\n");
        }else{  
            printf("destination: %s, path: %s, interface: %s, method:%s, parameters:%s\n",command.destination,
            command.path,command.interface,command.method,command.parameters);
        }
        free(current_line);
        free_command_buffer(&command);
    }*/
}
