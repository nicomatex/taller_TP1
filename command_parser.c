#include <string.h>
#include <stdbool.h>

#include "command_parser.h"


bool parse_command(command_t* command, char* command_str){
    const char delim[] = " ";
    char* token;

    token = strtok(command_str,delim);
    if(!token) return false;

    command->destination = strdup(token);

    token = strtok(NULL,delim);
    if(!token) return false;

    command->path = strdup(token);

    token = strtok(NULL,delim);
    if(!token) return false;

    command->interface = strdup(token);

    token = strtok(NULL,delim);
    if(!token) return false;

    command->method = strdup(token);

    token = strtok(NULL,delim);
    if(!token) return false;

    command->parameters = strdup(token);

    return true;
}

void free_command_buffer(command_t* command){
    if(command->destination) free(command->destination);
    if(command->interface) free(command->interface);
    if(command->method) free(command->method);
    if(command->parameters) free(command->parameters);
    if(command->path) free(command->path);
}