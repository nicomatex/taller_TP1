#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "file_streamer.h"
#include "command_parser.h"
#include "command_serializator.h"

void parsear_comando(char* string,void* context){
    command_t command;
    command_create(&command);

    if(!command_parse(string,&command)){
        fprintf(stderr,"Comando no valido\n");
        return;
    }
    command.msg_id = 8;
    size_t msg_size;
    unsigned char* message = generate_dbus_message(&command,&msg_size);
    
    
    command_t decoded_command;
    command_create(&decoded_command);
    decode_dbus_message(message,&decoded_command);
    free(message);

    printf("Destino: %s, Path: %s, Interfaz: %s, Metodo :%s\n",decoded_command.destination,decoded_command.path,decoded_command.interface,decoded_command.method);
    
    if(decoded_command.signature_param_count > 0){
        printf("Signature parametros: %s\n",decoded_command.signature_parameters);
    }
    command_destroy(&command);
    command_destroy(&decoded_command);
}

int main(void){
    file_streamer_t file_streamer;
    file_streamer_create(&file_streamer,stdin,parsear_comando,'\n');
    file_streamer_run(&file_streamer,NULL);
    file_streamer_destroy(&file_streamer);
    
    return 0;
}