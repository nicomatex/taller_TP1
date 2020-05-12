#include "common_dbus_protocol.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define PARAMS_INIT_SIZE 32
#define PARAMS_SCALE_FACTOR 2

#define BASE_PARAM_COUNT 4
#define EOS 1

#define ENDIAN 'l'
#define MSG_TYPE 0x01
#define FLAGS 0x00
#define PROTOCOL_VER 0X01

#define ARG_TYPE_PATH 0x01
#define ARG_TYPE_DEST 0x06
#define ARG_TYPE_INTERFACE 0x02
#define ARG_TYPE_METHOD 0x03
#define ARG_TYPE_SIGN 0x09

#define OVERHEAD_SIZE 16
#define MAGIC_BYTE 0x01

#define INITIAL_SIGNATURE_PARAMS 20

#define READ_START 4

enum param{DEST, PATH, INTERFACE, METHOD, SIGNATURE_PARAMS};

void command_create(command_t* command){
    command->destination = NULL;
    command->path = NULL;
    command->interface = NULL;
    command->method = NULL;
    command->signature_parameters = NULL;
    command->signature_param_count = 0;
    command->msg_id = 0;
}

void command_destroy(command_t* command){
    if ( command->destination ) free(command->destination);
    if ( command->interface ) free(command->interface);
    if ( command->method ) free(command->method);
    if ( command->signature_parameters ) free(command->signature_parameters);
    if ( command->path ) free(command->path);
}

static bool is_bigendian(){
    int n = 1;
    /*Desreferencia el byte menos significativo
    y se fija si es 1. Si lo es, entonces el sistema
    es little endian.*/
    return !(*(char*)&n == 1);
}

static size_t add_padding(size_t size){
    if ( size%8 == 0 ) return 0;
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

    if ( command->signature_param_count>0 ){
        header_size += 8;
        header_size += (command->signature_param_count + EOS) +
                         add_padding(command->signature_param_count + EOS);
    }

    header_size += header_size%8;

    return header_size;
}

uint32_t calculate_body_size(command_t* command){
    size_t body_size = 0;

    //Para los enteros que representan la longitud
    body_size += sizeof(uint32_t) * command->signature_param_count;

    //Para los \0 del final de cada cadena
    body_size += command->signature_param_count; 

    //Para no contar las comas
    body_size += (strlen(command->signature_parameters) - 
                    command->signature_param_count + 1); 
    return (uint32_t)body_size;
}

/*Devuelve la cantidad de bytes que escribio en el header*/
static size_t add_parameter_info(unsigned char* header,char* value,
                                enum param parameter,size_t position){
    size_t start_position = position;
    uint32_t param_size = (uint32_t)strlen(value);
    char type = 0;
    char data_type[2];
    data_type[1] = '\0';

    switch ( parameter ){
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
    
    uint32_t encoded_param_size = param_size;
    if ( is_bigendian() ) encoded_param_size = __builtin_bswap32(param_size);
    header[position++] = type;
    header[position++] = MAGIC_BYTE;
    memcpy(&header[position],&data_type[0],2);
    position += 2;
    memcpy(&header[position],&encoded_param_size,sizeof(uint32_t));
    position += sizeof(uint32_t);
    memcpy(&header[position],value,param_size);
    position += param_size;

    size_t nulls_to_add = 1;
    nulls_to_add += add_padding((size_t)param_size + 1);
    position += nulls_to_add;
    
    return position - start_position;
}

unsigned char* generate_header(command_t* command, size_t* header_size){
    /*TODO: Pasar los uint32_t a little endian*/
    *header_size = calculate_header_size(command);
    unsigned char* header = malloc(*header_size);
    if ( !header ){
        fprintf(stderr,"Error reservando memoria para el header.\n");
        return NULL;
    }
    memset(header,0,*header_size);
    if ( !header ) return NULL;

    header[POS_ENDIAN] = ENDIAN;
    header[POS_MSG_TYPE] = MSG_TYPE;
    header[POS_FLAGS] = FLAGS;
    header[POS_PROTOCOL_VER] = PROTOCOL_VER;

    uint32_t body_size = 0;

    if ( command->signature_param_count > 0 ) 
        body_size = calculate_body_size(command);

    uint32_t encoded_msg_id = command->msg_id;
    if ( is_bigendian() ){
        body_size = __builtin_bswap32(body_size);
        encoded_msg_id = __builtin_bswap32(command->msg_id);
    } 

    memcpy(&header[POS_BODY_SIZE],&body_size,sizeof(uint32_t));
    memcpy(&header[POS_SERIAL_NUMBER],&encoded_msg_id,sizeof(uint32_t));

    uint32_t array_size = (uint32_t)*header_size - OVERHEAD_SIZE;
    if ( is_bigendian() ) array_size = __builtin_bswap32(array_size);

    memcpy(&header[POS_ARRAY_SIZE],&array_size,sizeof(uint32_t));

    size_t position = POS_ARRAY_START;
    position += add_parameter_info(header,
                command->destination,DEST,position);
    position += add_parameter_info(header,
                command->path,PATH,position);
    position += add_parameter_info(header,
                command->interface,INTERFACE,position);
    position += add_parameter_info(header,
                command->method,METHOD,position);

    if ( command->signature_param_count > 0 ){
        char* signature_params_string = malloc(command->signature_param_count 
                                        + 1); //Para el \0
        if ( !signature_params_string ){
            fprintf(stderr,"Error de memoria\n");
            free(header);
            return NULL;
        }
        size_t i = 0;

        for ( i = 0; i<command->signature_param_count; i++ ) 
            signature_params_string[i] = 's';

        signature_params_string[i] = '\0';
        position += add_parameter_info(header,&signature_params_string[0],
                    SIGNATURE_PARAMS,position);
        free(signature_params_string);
    }
    return header;
}

unsigned char* generate_body(command_t* command,size_t* body_size){
    *body_size = calculate_body_size(command);
    unsigned char* body = malloc(*body_size);
    if ( !body ){
        fprintf(stderr,"Error reservando memoria para el cuerpo.\b");
        return NULL;
    }
    size_t params_index = 0;
    size_t body_index = sizeof(uint32_t);

    uint32_t current_param_lenght = 0;
    while ( command->signature_parameters[params_index] != '\0' ){
        body[body_index++] = command->signature_parameters[params_index++];
        current_param_lenght++;
        char current_char = command->signature_parameters[params_index];

        if ( current_char == SIGN_PARAM_SEP || current_char == '\0' ){
            uint32_t encoded_current_param_length = current_param_lenght;
            if ( is_bigendian() ){
                encoded_current_param_length = 
                __builtin_bswap32(current_param_lenght);
            }
            memcpy(&body[body_index-current_param_lenght-sizeof(uint32_t)],
            &encoded_current_param_length,sizeof(uint32_t));

            current_param_lenght = 0;
            body[body_index++] = '\0';
            body_index += sizeof(uint32_t);
            if ( current_char == '\0' ) break;
            params_index++;
        }
    }
    return body;
}

static char* decode_string(unsigned  char* message,size_t* position){
    size_t length = strlen((char*)&message[*position]);
    char* str = malloc(length+1);
    memcpy(str,&message[*position],length+1);
    *position += strlen(str) + 1;
    return str;
}

static uint32_t decode_int(unsigned char* message,size_t* position){
    uint32_t integer;
    memcpy(&integer,&message[*position],sizeof(uint32_t));
    if ( is_bigendian() ){
        integer = __builtin_bswap32(integer);
    }
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
    if ( !header ){
        return NULL;
    }
    *msg_size = header_size;

    if ( command->signature_param_count == 0 ){
        return header;
    }
    unsigned char* body = generate_body(command,&body_size);
    if ( !body ){
        free(header);
        return NULL;
    }
    *msg_size+= body_size;
    size_t message_size = body_size + header_size;

    unsigned char* message = malloc(message_size*sizeof(char));
    if ( !message ){
        free(header);
        free(body);
        fprintf(stderr,"Error reservando memoria para el mensaje.\n");
        return NULL;
    }
    memcpy(&message[0],&header[0],header_size);
    memcpy(&message[header_size],&body[0],body_size);
    free(header);
    free(body);
    return message;
}

void decode_body(unsigned char* message, command_t* command,
     size_t position, uint32_t body_size){
    if ( command->signature_param_count < 1 ) return;
    uint32_t body_end_position = (uint32_t)position + body_size;
    char* signature_params = malloc(PARAMS_INIT_SIZE*sizeof(char));
    size_t params_max_size = PARAMS_INIT_SIZE;
    size_t params_size = 0;
    while ( position < body_end_position ){
        size_t current_param_size = decode_int(message,&position);
        char* current_param = decode_string(message,&position);

        if ( params_size + current_param_size > params_max_size ){
            char* new_signature_params = realloc(signature_params,
                                (params_size + current_param_size)*
                                PARAMS_SCALE_FACTOR);

            if ( !new_signature_params ){
                free(signature_params);
                return;
            }else{
                signature_params = new_signature_params;
            }
            params_max_size = (params_size + current_param_size)*
                                PARAMS_SCALE_FACTOR;
        }
        
        memcpy(&signature_params[params_size],
        &current_param[0],current_param_size);

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

    while ( position < array_end_position ){
        char arg_type = decode_byte(message,&position);

        //Avanza 3 bytes para saltear bytes que no necesita leer.
        for ( int i = 0; i < 3; i++ ) position++; 

        uint32_t data_length = decode_int(message,&position);
        char* data = decode_string(message,&position);
        size_t padding = add_padding(data_length + 1);
        for ( int i = 0; i < padding; i++ ) position++; //Salteo el padding.
        switch ( arg_type ){
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

