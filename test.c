#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "file_streamer.h"
#include "command_parser.h"

void printear(char* string,void* context){
    printf("%s\n",string);
}

int main(void){
    file_streamer_t file_streamer;
    file_streamer_create(&file_streamer,stdin,printear,'\n');
    file_streamer_run(&file_streamer,NULL);
    file_streamer_destroy(&file_streamer);
    
    /*command_t command;
    command_create(&command);
    command_parse("taller.server /tp1/server com.taller.tp1 saludar()\n",&command);
    printf("destino: %s, path: %s, interfaz: %s, metodo: %s, parametros: %s\n",command.destination,command.path,command.interface,command.method,command.parameters);
    if(! command.parameters){
        printf("El comando no tenia params\n");
    }
    command_destroy(&command);*/
    return 0;
}