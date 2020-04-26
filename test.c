#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "file_streamer.h"
#include "command_parser.h"
#include "command_serializator.h"

void parsear_comando(char* string,void* context){
    command_t command;
    command_create(&command);
    command_parse(string,&command);
    printf("destino: %s, path: %s, interfaz: %s, metodo: %s, parametros: %s\n",command.destination,command.path,command.interface,command.method,command.signature_parameters);
    
    if(!command.signature_parameters){
        printf("El comando no tenia params\n");
    }
    
    size_t header_size = 0;
    char* header = NULL;
    header = generate_header(&command,0,&header_size);
    
    printf("El size del header es %ld\n",header_size);
    
    printf("Header: ");
    for(size_t i = 0;i<header_size;i++){
        printf("%.2x ",header[i]);
    }
    printf("\n");
    free(header);
    command_destroy(&command);
}

int main(void){
    file_streamer_t file_streamer;
    file_streamer_create(&file_streamer,stdin,parsear_comando,'\n');
    file_streamer_run(&file_streamer,NULL);
    file_streamer_destroy(&file_streamer);
    
    return 0;
}