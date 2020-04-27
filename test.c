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

    size_t msg_size;
    unsigned char* message = generate_dbus_message(&command,8,&msg_size);

    printf("===== MENSAJE ====\n");
    for(size_t i = 0;i<msg_size;i++){
        printf("%.2x ",message[i]);
    }
    printf("\n");
    free(message);
    command_destroy(&command);
}

int main(void){
    file_streamer_t file_streamer;
    file_streamer_create(&file_streamer,stdin,parsear_comando,'\n');
    file_streamer_run(&file_streamer,NULL);
    file_streamer_destroy(&file_streamer);
    
    return 0;
}