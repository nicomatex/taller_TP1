#ifndef COMMAND_SERIALIZATOR_H
#define COMMAND_SERIALIZATOR_H
#include "command_parser.h"
#include <stdint.h>

/*Recibe un command_t lleno con los datos del comando apropiado, un numero de serie, y un puntero a entero msg_size 
en el cual almacena el largo del mensaje en bytes. Devuelve un arreglo de bytes con el mensaje serializado en el 
formato de DBUs, listo para enviar. La memoria debe liberada luego de su uso.*/
unsigned char* generate_dbus_message(command_t* command, uint32_t serial_number,size_t* msg_size);
#endif