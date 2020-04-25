
#include "file_streamer.h"
#include <stdlib.h>
#define BUFFER_INITIAL_SIZE 32

void file_streamer_create(file_streamer_t* file_streamer,FILE* archivo,callback_t callback,char separador){
    file_streamer->archivo = archivo;
    file_streamer->callback = callback;
    file_streamer->separador = separador;
}

int file_streamer_run(file_streamer_t* file_streamer,void* context){
    size_t buffer_size = BUFFER_INITIAL_SIZE;
    char* buffer = malloc(sizeof(char)*buffer_size);

    if(!buffer){
        return -1;
    }

    size_t read_bytes = 0;
    int total_read_bytes = 0;
    int read_byte = 0;
    do{
        read_byte = fgetc(file_streamer->archivo);
        total_read_bytes++;
        /*Si llego al salto de linea, enviarle la linea a callback y empezar de nuevo.*/
        if(read_byte == file_streamer->separador){
            buffer[read_bytes] = '\0';
            file_streamer->callback(buffer,context);
            read_bytes = 0;
            continue;
        }
        buffer[read_bytes] = read_byte;
        read_bytes++;

        /*Si se lleno el buffer, duplicar su size.*/
        if (read_bytes >= buffer_size - 1){ 
            buffer = realloc(buffer, 2 * buffer_size);
            if(!buffer){
                return -1;
            }
            buffer_size *= 2;
        }
           
    }while(read_byte != EOF);

    free(buffer);
    return total_read_bytes;
}   

void file_streamer_destroy(file_streamer_t* file_streamer){
    ;
}