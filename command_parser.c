#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "command_parser.h"

#define FORMAT "%s %s %s %[^(](%[^)\n])"
#define MIN_PARAMS 4

#define PARAM_SEP ' '
#define METHOD_PARAM_SEP '('
#define SIGNATURE_PARAM_SEP ')'

typedef struct parameter_sizes{
    size_t dest_size;
    size_t path_size;
    size_t interface_size;
    size_t method_size;
    size_t signature_params_size;
}parameter_sizes_t;

static size_t get_parameter_size(char* line,size_t* i,char sep){
    size_t param_size = 0;

    while(line[*i] != sep && line[*i] != '\0'){
        *i+=1; 
        param_size++;
    }
    return param_size;
}

static bool calculate_parameter_sizes(char* line,parameter_sizes_t* parameter_sizes){
    size_t i = 0;
    size_t param_size;
    
    param_size = get_parameter_size(line,&i,PARAM_SEP);
    if(line[i] == '\0') return false;
    parameter_sizes->dest_size = param_size;
    i++;

    param_size = get_parameter_size(line,&i,PARAM_SEP);
    if(line[i] == '\0') return false;
    parameter_sizes->path_size = param_size;
    i++;

    param_size = get_parameter_size(line,&i,PARAM_SEP);
    if(line[i] == '\0') return false;
    parameter_sizes->interface_size = param_size;
    i++;

    param_size = get_parameter_size(line,&i,METHOD_PARAM_SEP);
    if(line[i] == '\0') return false;
    parameter_sizes->method_size = param_size;
    i++;

    param_size = get_parameter_size(line,&i,SIGNATURE_PARAM_SEP);
    if(line[i] == '\0') return false;
    parameter_sizes->signature_params_size = param_size;

    return true;
}

static void allocate_parameter_space(parameter_sizes_t* parameter_sizes, command_t* command){
    command->destination = malloc((parameter_sizes->dest_size + 1)*sizeof(char));
    command->path = malloc((parameter_sizes->path_size + 1)*sizeof(char));
    command->interface = malloc((parameter_sizes->interface_size + 1)*sizeof(char));
    command->method = malloc((parameter_sizes->method_size + 1)*sizeof(char));

    if(parameter_sizes->signature_params_size > 0){
        command->signature_parameters = malloc((parameter_sizes->signature_params_size + 1)*sizeof(char));
    }
}

bool command_parse(char* line, command_t* command){
    int read_params = 0;
    parameter_sizes_t parameter_sizes;

    if(!calculate_parameter_sizes(line,&parameter_sizes)) return false;
    allocate_parameter_space(&parameter_sizes,command);

    read_params = sscanf(line,FORMAT, command->destination, command->path, command->interface, command->method,command->signature_parameters);

    if(read_params < MIN_PARAMS){
        command_destroy(command);
        return false;
    }

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
    return true;
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