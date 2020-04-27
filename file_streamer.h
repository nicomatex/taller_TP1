#ifndef FILE_STREAMER_H
#define FILE_STREAMER_H

#include <stdio.h>

typedef void (*callback_t)(char*,void*);

typedef struct file_streamer{
    FILE* file;
    callback_t callback;
    char separator;
} file_streamer_t;

/*Crea un file streamer. Recibe un puntero a file_streamer_t en el cual sera inicializado el streamer,
un FILE DESCRIPTOR archivo, y una funcion de callback.*/
void file_streamer_create(file_streamer_t* file_streamer,FILE* archivo,callback_t callback,char separador);

/*Pone a correr el file streamer. El context esta siempre
disponible para la funcion de callback.
*/
int file_streamer_run(file_streamer_t* file_streamer,void* context);

/*Destruye un file_streamer que fue creado.*/
void file_streamer_destroy(file_streamer_t* file_streamer);

#endif