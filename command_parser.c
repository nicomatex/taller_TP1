#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "command_parser.h"

bool command_parse(char* line, command_t* command){
    sscanf(line,"%ms %ms %ms %m[^(](%m[^)\n])", &command->destination, &command->path, &command->interface, &command->method,&command->parameters);
}

void command_create(command_t* command){
    command->destination = NULL;
    command->path = NULL;
    command->interface = NULL;
    command->method = NULL;
    command->parameters = NULL;
}

void command_destroy(command_t* command){
    if(command->destination) free(command->destination);
    if(command->interface) free(command->interface);
    if(command->method) free(command->method);
    if(command->parameters) free(command->parameters);
    if(command->path) free(command->path);
}