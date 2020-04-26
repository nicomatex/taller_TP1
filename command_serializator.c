#include <string.h>
#include "command_parser.h"
#include "command_serializator.h"
#include <stdio.h>
#include <stdint.h>

#define PARAMETER_SEP ','
#define BASE_PARAM_COUNT 4
#define EOS 1
#define BASE_HEADER_SIZE 16

#define ENDIAN 'l'
#define MSG_TYPE 0x01
#define FLAGS 0x00
#define PROTOCOL_VER 0X01

#define POS_ENDIAN 0
#define POS_MSG_TYPE 1
#define POS_FLAGS 2
#define POS_PROTOCOL_VER 3
#define POS_BODY_SIZE 4
#define POS_SERIAL_NUMBER 8
#define POS_ARRAY_SIZE 12
#define POS_ARRAY_START 16

#define ARG_TYPE_PATH 0x01
#define ARG_TYPE_DEST 0x06
#define ARG_TYPE_INTERFACE 0x02
#define ARG_TYPE_METHOD 0x03
#define ARG_TYPE_SIGN 0x09

#define OVERHEAD_SIZE 16
#define MAGIC_BYTE 0x01

#define MAX_SIGNATURE_PARAMS 20

enum param{DEST, PATH, INTERFACE, METHOD, SIGNATURE_PARAMS};

static size_t add_padding(size_t size){
    if(size%8 == 0) return 0;
    return 8 - size%8;
}

static size_t calculate_header_size(command_t* command){

    size_t string_size = 0;
    size_t header_size = BASE_HEADER_SIZE + 8*BASE_PARAM_COUNT;

    string_size = strlen(command->destination) + EOS;
    header_size += string_size + add_padding(string_size); /*Le sumo lo que le falta para ser multiplo de 8*/

    string_size = strlen(command->path) + EOS;
    header_size += string_size + add_padding(string_size);

    string_size = strlen(command->interface) + EOS;
    header_size += string_size + add_padding(string_size);

    string_size = strlen(command->method) + EOS;
    header_size += string_size + add_padding(string_size);

    if(command->signature_param_count>0){
        header_size += 8;
        header_size += (command->signature_param_count + EOS) + add_padding(command->signature_param_count + EOS);
    }

    header_size += header_size%8;

    return header_size;
}

/*Devuelve la cantidad de bytes que escribio en el header*/
static size_t add_parameter_info(char* header,char* value,enum param parameter,size_t position){
    size_t start_position = position;
    uint32_t param_size = (uint32_t)strlen(value);
    char type = 0;
    char data_type[2];
    data_type[1] = '\0';

    switch(parameter){
        case PATH:
            type = ARG_TYPE_PATH;
            data_type[0] = 'o';
            break;
        case DEST:
            type = ARG_TYPE_DEST;
            data_type[0] = 's';
            break;
        case INTERFACE:
            type = ARG_TYPE_INTERFACE;
            data_type[0] = 's';
            break;
        case METHOD:
            type = ARG_TYPE_METHOD;
            data_type[0] = 's';
            break;
        case SIGNATURE_PARAMS:
            type = ARG_TYPE_SIGN;
            data_type[0] = 'g';
            break;
    }
    
    header[position++] = type;
    header[position++] = MAGIC_BYTE;
    memcpy(&header[position],&data_type[0],2);
    position += 2;
    memcpy(&header[position],&param_size,sizeof(uint32_t));
    position += sizeof(uint32_t);
    memcpy(&header[position],value,param_size);
    position += param_size;

    size_t nulls_to_add = 1;
    nulls_to_add += add_padding((size_t)param_size + 1);
    position += nulls_to_add;
    
    return position - start_position;
}

unsigned char* generate_header(command_t* command, uint32_t serial_number, size_t* header_size){
    /*TODO: Pasar los uint32_t a little endian*/
    *header_size = calculate_header_size(command);
    unsigned char* header = malloc(*header_size);
    memset(header,0,*header_size);
    if(!header) return NULL;

    header[POS_ENDIAN] = ENDIAN;
    header[POS_MSG_TYPE] = MSG_TYPE;
    header[POS_FLAGS] = FLAGS;
    header[POS_PROTOCOL_VER] = PROTOCOL_VER;

    uint32_t body_size = 8; /*TODO: cambiar esto cuando implemente la generacion del body*/
    memcpy(&header[POS_BODY_SIZE],&body_size,sizeof(uint32_t));
    memcpy(&header[POS_SERIAL_NUMBER],&serial_number,sizeof(uint32_t));

    uint32_t array_size = (uint32_t)*header_size - OVERHEAD_SIZE;
    memcpy(&header[POS_ARRAY_SIZE],&array_size,sizeof(uint32_t));

    size_t position = POS_ARRAY_START;
    position += add_parameter_info(header,command->destination,DEST,position);
    position += add_parameter_info(header,command->path,PATH,position);
    position += add_parameter_info(header,command->interface,INTERFACE,position);
    position += add_parameter_info(header,command->method,METHOD,position);

    if(command->signature_param_count > 0){
        char signature_params_string[MAX_SIGNATURE_PARAMS] = "";
        size_t i = 0;
        for(i = 0;i<command->signature_param_count;i++) signature_params_string[i] = 's';
        position += add_parameter_info(header,&signature_params_string[0],SIGNATURE_PARAMS,position);
    }
    printf("termine escribiendo %ld\n",position);
    return header;
}