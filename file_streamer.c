
#include "file_streamer.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define FILE_BUFFER_SIZE 32
#define LINE_BUFFER_INITIAL_SIZE 32

#define BUF_SCALE_FACTOR 2

#define CHAR_ENDOFSTRING '\0'

void file_streamer_create(file_streamer_t* file_streamer,FILE* archivo,callback_t callback,char separador){
    file_streamer->file = archivo;
    file_streamer->callback = callback;
    file_streamer->separator = separador;
}

char* resize_buffer(char* old_buffer,size_t old_size){ /*Es una especie de realloc, pero no confio en realloc.*/
    char* new_buffer = malloc(BUF_SCALE_FACTOR*old_size);
    if(!new_buffer){
        free(old_buffer);
        return NULL; /*Si falla el malloc*/
    }
    memset(new_buffer,0,BUF_SCALE_FACTOR*old_size);
    memcpy(new_buffer,old_buffer,old_size);
    free(old_buffer);
    return new_buffer;
}

int file_streamer_run(file_streamer_t* file_streamer,void* context){
    char file_buffer[FILE_BUFFER_SIZE] = "";
    char* line_buffer = malloc(sizeof(char)*LINE_BUFFER_INITIAL_SIZE);
    if(!line_buffer) return -1; /*Si falla el malloc*/

    size_t line_buffer_size = LINE_BUFFER_INITIAL_SIZE;
    size_t current_line_size = 0;

    while(!feof(file_streamer->file)){
        size_t bytes_read = fread(file_buffer,1,FILE_BUFFER_SIZE,file_streamer->file);
        size_t i = 0;
        for(i = 0;i < bytes_read;i++){ //Se recorre todo el buffer
            line_buffer[current_line_size] = file_buffer[i]; //Se pasa cada byte al buffer de linea
            current_line_size++;

            if(current_line_size == line_buffer_size - 1){ /*Si se llena el buffer de linea, duplico su tamanio*/
                char* new_line_buffer = resize_buffer(line_buffer,line_buffer_size);
                if(!new_line_buffer) return -1;
                line_buffer_size*= BUF_SCALE_FACTOR;
                line_buffer = new_line_buffer;
            }

            if(file_buffer[i] == file_streamer->separator){
                line_buffer[current_line_size - 1] = CHAR_ENDOFSTRING;
                file_streamer->callback(line_buffer,context);
                current_line_size = 0;
            }
        }
    }
    free(line_buffer);
    return 0;
}   

void file_streamer_destroy(file_streamer_t* file_streamer){
    ;
}