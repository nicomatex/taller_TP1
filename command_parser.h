#include <stdlib.h>

typedef struct command{
    char* destination;
    char* path;
    char* interface;
    char* method;
    char* parameters;
} command_t;

/*Recibe un puntero a struct command_t y una string command_str, parsea la string y rellena 
los campos del struct en el orden especificado. Devuelve True si el proceso fue exitoso, 
false en caso contrario.*/
bool parse_command(command_t* command, char* command_str);

/*Libera la memoria utilizada para almacenar el comando en la estructura command_t
pasada por parametro.*/
void free_command_buffer(command_t* command);