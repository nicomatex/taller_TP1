#include "server.h"
#include "common_dbus_protocol.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define SV_RESPONSE "OK\n"
#define SV_BUFFER_BASE_SIZE 32
#define RESPONSE_SIZE 3

static bool is_bigendian(){
    int n = 1;
    /*Desreferencia el byte menos significativo
    y se fija si es 1. Si lo es, entonces el sistema
    es little endian.*/
    return !(*(char*)&n == 1);
}

static uint32_t decode_int(unsigned char* message,size_t position){
    uint32_t integer;
    memcpy(&integer,&message[position],sizeof(uint32_t));
    if ( is_bigendian() ){
        integer = __builtin_bswap32(integer);
    }
    return integer;
}

static char* get_token(char* str, char sep, char** saveptr){
    if ( str ){
        *saveptr = str;
    }
    size_t i = 0;
    while ( (*saveptr)[i] != sep && (*saveptr)[i] != '\0' ) i++;
    if ( (*saveptr)[i] == '\0' ) return NULL;
    
    char* return_address = *saveptr;
    (*saveptr)[i] = '\0';
    *saveptr += i+1;
    return return_address;
}

static void print_command(command_t* command){
    printf("* Id: 0x%.8x\n",command->msg_id);
    printf("* Destino: %s\n",command->destination);
    printf("* Ruta: %s\n",command->path);
    printf("* Interfaz: %s\n",command->interface);
    printf("* Metodo: %s\n",command->method);

    if ( command->signature_param_count > 0 ){
        printf("* Parametros:\n");
        char* parameter;
        char* saveptr;
        parameter = get_token(command->signature_parameters,
                            SIGN_PARAM_SEP,&saveptr);

        while ( parameter != NULL ){
            printf("    * %s\n",parameter);
            parameter = get_token(NULL,SIGN_PARAM_SEP,&saveptr);
        }
    }
    printf("\n");
}

void dbus_server_run(server_t* server){
    unsigned char* buffer = malloc(BASE_HEADER_SIZE*sizeof(char));
    size_t buffer_max_size = SV_BUFFER_BASE_SIZE;
    size_t bytes_recieved = server_recv_msg(server,&buffer[0],BASE_HEADER_SIZE);

    while ( bytes_recieved > 0 ){
        size_t param_array_size = decode_int(buffer,POS_ARRAY_SIZE);
        size_t body_size = decode_int(buffer,POS_BODY_SIZE);

        if ( BASE_HEADER_SIZE + param_array_size + body_size 
            >= buffer_max_size ){
            unsigned char* new_buffer = realloc(buffer,BASE_HEADER_SIZE +
                                        param_array_size + body_size);
            if ( !new_buffer ){
                free(buffer);
                return;
            }else{
                buffer = new_buffer;
            }
            buffer_max_size = BASE_HEADER_SIZE + param_array_size + body_size;
        }

        server_recv_msg(server,&buffer[BASE_HEADER_SIZE],
        param_array_size + body_size);

        char* response = SV_RESPONSE;
        server_send_msg(server,(unsigned char*)response,RESPONSE_SIZE);

        command_t command_recieved;
        command_create(&command_recieved);
        decode_dbus_message(&buffer[0],&command_recieved);
        print_command(&command_recieved);
        command_destroy(&command_recieved);

        bytes_recieved = server_recv_msg(server,&buffer[0],BASE_HEADER_SIZE);
    }

    free(buffer);
}
