#include "common_dbus_parser.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

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

    while ( line[*i] != sep && line[*i] != '\0' ){
        *i+=1; 
        param_size++;
    }
    return param_size;
}

static bool calculate_parameter_sizes(char* line,
            parameter_sizes_t* parameter_sizes){
    size_t i = 0;
    size_t param_size;
    
    param_size = get_parameter_size(line,&i,PARAM_SEP);
    if ( line[i] == '\0' ) return false;
    parameter_sizes->dest_size = param_size;
    i++;

    param_size = get_parameter_size(line,&i,PARAM_SEP);
    if ( line[i] == '\0' ) return false;
    parameter_sizes->path_size = param_size;
    i++;

    param_size = get_parameter_size(line,&i,PARAM_SEP);
    if ( line[i] == '\0' ) return false;
    parameter_sizes->interface_size = param_size;
    i++;

    param_size = get_parameter_size(line,&i,METHOD_PARAM_SEP);
    if ( line[i] == '\0' ) return false;
    parameter_sizes->method_size = param_size;
    i++;

    param_size = get_parameter_size(line,&i,SIGNATURE_PARAM_SEP);
    if ( line[i] == '\0' ) return false;
    parameter_sizes->signature_params_size = param_size;

    return true;
}

static int allocate_parameter_space(parameter_sizes_t* parameter_sizes,
             command_t* command){
    command->destination = malloc(parameter_sizes->dest_size + 1);
    if ( !command->destination ) return -1;

    command->path = malloc(parameter_sizes->path_size + 1);
    if ( !command->path ){
        command_destroy(command);
        return -1;
    }
    command->interface = malloc(parameter_sizes->interface_size + 1);
    if ( !command->interface ){
        command_destroy(command);
        return -1;
    }
    command->method = malloc(parameter_sizes->method_size + 1);
    if ( !command->method ){
        command_destroy(command);
        return -1;
    }
    if ( parameter_sizes->signature_params_size > 0 ){
        command->signature_parameters = 
        malloc(parameter_sizes->signature_params_size + 1);
        if ( !command->signature_parameters ){
            command_destroy(command);
            return -1;
        }
    }
    return 0;
}

static void fill_single_parameter(char* line,size_t position,
            char* param_storage,size_t size){
    for ( size_t i=0; i<size; i++ ){
        param_storage[i] = line[position+i];
    }
    param_storage[size] = '\0';
}

static void fill_parameters(command_t* command,
            parameter_sizes_t* parameter_sizes,char* line){
    size_t position = 0;

    fill_single_parameter(line,position,command->destination,
    parameter_sizes->dest_size);
    position += parameter_sizes->dest_size+1;

    fill_single_parameter(line,position,command->path,
    parameter_sizes->path_size);
    position += parameter_sizes->path_size+1;

    fill_single_parameter(line,position,command->interface,
    parameter_sizes->interface_size);
    position += parameter_sizes->interface_size+1;

    fill_single_parameter(line,position,command->method,
    parameter_sizes->method_size);
    position += parameter_sizes->method_size+1;

    if ( parameter_sizes->signature_params_size > 0 ){
        fill_single_parameter(line,position,command->signature_parameters,
        parameter_sizes->signature_params_size);
    } else {
        command->signature_parameters = NULL;
    }
}

bool command_parse(char* line, command_t* command){
    parameter_sizes_t parameter_sizes;

    if ( !calculate_parameter_sizes(line,&parameter_sizes) ) return false;
    if ( allocate_parameter_space(&parameter_sizes,command) < 0 ) return false;

    fill_parameters(command,&parameter_sizes,line);

    size_t signature_param_count = 0;
    size_t i = 0;

    if ( command->signature_parameters ){
        signature_param_count = 1;
        while ( command->signature_parameters[i] != '\0' ){
            if ( command->signature_parameters[i] == ',' ) 
                signature_param_count++;
        i++;
        }
        command->signature_param_count = signature_param_count;
    }
    return true;
}

