#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "command_parser.h"

bool command_parse(char* line, command_t* command){
    sscanf(line,"%ms %ms %ms %m[^(](%m[^)\n])", &command->destination, &command->path, &command->interface, &command->method,&command->signature_parameters);
    
    size_t signature_param_count = 0;
    size_t i = 0;

    if(command->signature_parameters){ /*Esto es para los signature parameters*/
        signature_param_count = 1;
        while(command->signature_parameters[i] != '\0'){
            if(command->signature_parameters[i] == ',') signature_param_count++;
        i++;
        }
        command->signature_param_count = signature_param_count;
    }
}

void command_create(command_t* command){
    command->destination = NULL;
    command->path = NULL;
    command->interface = NULL;
    command->method = NULL;
    command->signature_parameters = NULL;
    command->signature_param_count = 0;
}

void command_destroy(command_t* command){
    if(command->destination) free(command->destination);
    if(command->interface) free(command->interface);
    if(command->method) free(command->method);
    if(command->signature_parameters) free(command->signature_parameters);
    if(command->path) free(command->path);
}