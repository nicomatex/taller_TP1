#include "client.h"
#include "common_dbus_protocol.h"
#include "common_dbus_parser.h"

#include <stdio.h>

#define RESPONSE_SIZE 3

void dbus_parse_and_send(char* line,void* context){
    client_t* client = (client_t*) context;
    command_t command;
    command_create(&command);

    if ( !command_parse(line,&command) ){
        fprintf(stderr,"Comando no valido\n");
        return;
    }

    command.msg_id = client->current_msg_id;

    size_t msg_size;
    
    unsigned char* message = generate_dbus_message(&command,&msg_size);

    size_t bytes_sent = client_send_msg(client,message,msg_size);

    if ( bytes_sent == 0 ){
        free(message);
        command_destroy(&command);
        fprintf(stderr,"Conexion cerrada desde el servidor");
    }

    unsigned char response_buffer[RESPONSE_SIZE+1];
    size_t bytes_recieved = client_recv_msg(client,
                            &response_buffer[0],RESPONSE_SIZE);

    if ( bytes_recieved == 0 ){
        free(message);
        command_destroy(&command);
        fprintf(stderr,"Conexion cerrada desde el servidor");
    }

    response_buffer[RESPONSE_SIZE] = '\0';
    printf("0x%.8x: %s",client->current_msg_id,response_buffer);

    client->current_msg_id += 1;
    
    free(message);
    command_destroy(&command);
}
