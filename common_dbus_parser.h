#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "common_dbus_protocol.h"

/*Recibe un puntero a struct command_t y una string command_str, parsea la string y rellena 
los campos del struct en el orden especificado. Devuelve True si el proceso fue exitoso, 
false en caso contrario.*/
bool command_parse(char* command_str,command_t* command);


#endif
