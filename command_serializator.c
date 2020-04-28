#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include "command_parser.h"
#include "command_serializator.h"
#include <stdio.h>
#include <stdint.h>

#define PARAMS_INIT_SIZE 32
#define PARAMS_SCALE_FACTOR 2

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

#define READ_START 4

#define SIGN_PARAM_SEP ','

enum param{DEST, PATH, INTERFACE, METHOD, SIGNATURE_PARAMS};

static size_t add_padding(size_t size){
    if(size%8 == 0) return 0;
    return 8 - size%8;
}

static size_t calculate_header_size(command_t* command){

    size_t string_size = 0;
    size_t header_size = BASE_HEADER_SIZE + 8*BASE_PARAM_COUNT;

    string_size = strlen(command->destination) + EOS;
    header_size += string_size + add_padding(string_size); 

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

uint32_t calculate_body_size(command_t* command){
    size_t body_size = 0;
    body_size += sizeof(uint32_t) * command->signature_param_count; //Para los enteros de longitud
    body_size += command->signature_param_count; //Para los \0 del final de cada cadena
    body_size += (strlen(command->signature_parameters) - command->signature_param_count + 1); // Para no contar las comas
    return (uint32_t)body_size;
}

/*Devuelve la cantidad de bytes que escribio en el header*/
static size_t add_parameter_info(unsigned char* header,char* value,enum param parameter,size_t position){
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

static unsigned char* generate_header(command_t* command, size_t* header_size){
    /*TODO: Pasar los uint32_t a little endian*/
    *header_size = calculate_header_size(command);
    unsigned char* header = malloc(*header_size);
    memset(header,0,*header_size);
    if(!header) return NULL;

    header[POS_ENDIAN] = ENDIAN;
    header[POS_MSG_TYPE] = MSG_TYPE;
    header[POS_FLAGS] = FLAGS;
    header[POS_PROTOCOL_VER] = PROTOCOL_VER;

    uint32_t body_size = 0;
    if(command->signature_param_count > 0) body_size = calculate_body_size(command);
    memcpy(&header[POS_BODY_SIZE],&body_size,sizeof(uint32_t));
    memcpy(&header[POS_SERIAL_NUMBER],&command->msg_id,sizeof(uint32_t));

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
    return header;
}

static unsigned char* generate_body(command_t* command,size_t* body_size){
    *body_size = calculate_body_size(command);
    unsigned char* body = malloc(*body_size);
    size_t params_index = 0;
    size_t body_index = sizeof(uint32_t);

    uint32_t current_param_lenght = 0;
    while(command->signature_parameters[params_index] != '\0'){
        body[body_index++] = command->signature_parameters[params_index++];
        current_param_lenght++;

        char current_char = command->signature_parameters[params_index];
        if(current_char == SIGN_PARAM_SEP || current_char == '\0'){
            memcpy(&body[body_index-current_param_lenght-sizeof(uint32_t)],&current_param_lenght,sizeof(uint32_t));
            current_param_lenght = 0;
            body[body_index++] = '\0';
            body_index += sizeof(uint32_t);
            if(current_char == '\0') break;
            params_index++;
        }
    }
    return body;
}


static char* decode_string(unsigned  char* message,size_t* position){
    char* string = strdup((char*)&message[*position]);
    *position += strlen(string) + 1;
    return string;
}

static uint32_t decode_int(unsigned char* message,size_t* position){
    uint32_t integer;
    memcpy(&integer,&message[*position],sizeof(uint32_t));
    
    *position += sizeof(uint32_t);
    return integer;
}

static char decode_byte(unsigned  char* message,size_t* position){
    char byte; 
    byte = message[*position];
    *position += sizeof(char);
    return byte;
}

unsigned char* generate_dbus_message(command_t* command,size_t* msg_size){
    size_t body_size = 0;
    size_t header_size = 0;
    unsigned char* header = generate_header(command,&header_size);
    *msg_size = header_size;

    if(command->signature_param_count == 0){
        return header;
    }
    unsigned char* body = generate_body(command,&body_size);
    *msg_size+= body_size;
    size_t message_size = body_size + header_size;

    unsigned char* message = malloc(message_size*sizeof(char));
    
    memcpy(&message[0],&header[0],header_size);
    memcpy(&message[header_size],&body[0],body_size);
    free(header);
    free(body);
    return message;
}

void decode_body(unsigned char* message, command_t* command, size_t position, uint32_t body_size){
    if(command->signature_param_count < 1) return;
    uint32_t body_end_position = (uint32_t)position + body_size;
    char* signature_params = malloc(PARAMS_INIT_SIZE*sizeof(char));
    size_t params_max_size = PARAMS_INIT_SIZE;
    size_t params_size = 0;
    while(position < body_end_position){
        size_t current_param_size = decode_int(message,&position);
        char* current_param = decode_string(message,&position);
        if(params_size + current_param_size > params_max_size){
            signature_params = realloc(signature_params,PARAMS_SCALE_FACTOR*params_max_size);
        }
        memcpy(&signature_params[params_size],&current_param[0],current_param_size);
        free(current_param);
        signature_params[params_size+current_param_size] = ',';
        params_size += current_param_size + 1;
    }
    signature_params[params_size - 1] = '\0';
    command->signature_parameters = signature_params;
}

void decode_dbus_message(unsigned char* message, command_t* command){
    size_t position = READ_START;
    uint32_t body_size = decode_int(message,&position);
    command->msg_id = decode_int(message,&position);
    uint32_t array_size = decode_int(message,&position);
    
    uint32_t array_end_position = (uint32_t)position + array_size;

    while(position < array_end_position){
        char arg_type = decode_byte(message,&position);
        for(int i = 0;i < 3;i++) position++; //Avanza 3 bytes para saltear bytes que no necesita leer.
        uint32_t data_length = decode_int(message,&position);
        char* data = decode_string(message,&position);
        size_t padding = add_padding(data_length + 1);
        for(int i = 0;i < padding;i++) position++; //Salteo el padding.
        switch(arg_type){
            case ARG_TYPE_DEST:
                command->destination = data;
                break;
            case ARG_TYPE_PATH:
                command->path = data;
                break;
            case ARG_TYPE_METHOD:
                command->method = data;
                break;
            case ARG_TYPE_INTERFACE:
                command->interface = data;
                break;
            case ARG_TYPE_SIGN:
                command->signature_param_count = strlen(data);
                free(data);
                break;
        }
        decode_body(message, command, position, body_size);
    }
}

