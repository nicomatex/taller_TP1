#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct command{
    char* destination;
    char* path;
    char* interface;
    char* method;
    char* signature_parameters;
    size_t signature_param_count;
    uint32_t msg_id;
} command_t;

void command_create(command_t* command);

/*Recibe un puntero a struct command_t y una string command_str, parsea la string y rellena 
los campos del struct en el orden especificado. Devuelve True si el proceso fue exitoso, 
false en caso contrario.*/
bool command_parse(char* command_str,command_t* command);

/*Libera la memoria utilizada para almacenar el comando en la estructura command_t
pasada por parametro.*/
void command_destroy(command_t* command);
#endif