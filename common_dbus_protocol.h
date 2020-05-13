#ifndef __COMMON_DBUS_PROTOCOL_H
#define __COMMON_DBUS_PROTOCOL_H

#include <stdint.h>
#include <stdlib.h>

#define BASE_HEADER_SIZE 16

#define POS_ENDIAN 0
#define POS_MSG_TYPE 1
#define POS_FLAGS 2
#define POS_PROTOCOL_VER 3
#define POS_BODY_SIZE 4
#define POS_SERIAL_NUMBER 8
#define POS_ARRAY_SIZE 12
#define POS_ARRAY_START 16
#define SIGN_PARAM_SEP ','

typedef struct command{
    char* destination;
    char* path;
    char* interface;
    char* method;
    char* signature_parameters;
    size_t signature_param_count;
    uint32_t msg_id;
} command_t;

/*Recibe un command_t lleno con los datos del comando apropiado, un numero de serie, y un puntero a entero msg_size 
en el cual almacena el largo del mensaje en bytes. Devuelve un arreglo de bytes con el mensaje serializado en el 
formato de DBUs, listo para enviar. La memoria debe liberada luego de su uso.*/
unsigned char* generate_dbus_message(command_t* command,size_t* msg_size);


/*Recibe un mensaje codificado de DBUS, y almacena el resultado de la decodificacion del mismo en 
el command pasado por parametro.*/
void decode_dbus_message(unsigned char* message, command_t* command);

/* Inicializa el comando command pasado por parametro.*/
void command_create(command_t* command);

/*Libera la memoria utilizada para almacenar el comando en la estructura command_t
pasada por parametro.*/
void command_destroy(command_t* command);
#endif
