
#include "file_streamer.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define FILE_BUFFER_SIZE 32
#define LINE_BUFFER_INITIAL_SIZE 32

#define BUF_SCALE_FACTOR 3

#define CHAR_ENDOFSTRING '\0'

void file_streamer_create(file_streamer_t* file_streamer,
                        FILE* archivo,callback_t callback,char separador){
    file_streamer->file = archivo;
    file_streamer->callback = callback;
    file_streamer->separator = separador;
}

static int fill_line_buffer(file_streamer_t* file_streamer, void* context,
            size_t bytes_read,char** line_buffer,char* file_buffer,
            size_t* current_line_size,size_t *line_buffer_size){
    for ( size_t i = 0; i < bytes_read; i++ ){ //Se recorre todo el buffer
        //Se pasa cada byte al buffer de linea
        (*line_buffer)[*current_line_size] = file_buffer[i]; 
        (*current_line_size)++;

        /*Si se llena el buffer de linea, duplico su tamanio*/
        if ( (*current_line_size) == (*line_buffer_size) - 1 ){ 
            size_t new_buffer_size = (*line_buffer_size)*BUF_SCALE_FACTOR;
            char* new_line_buffer = realloc(*line_buffer,new_buffer_size);
            if ( !new_line_buffer ){
                return -1;
            } 
            (*line_buffer_size) = new_buffer_size;
            *line_buffer = new_line_buffer;
        }

        if ( file_buffer[i] == file_streamer->separator ){
            (*line_buffer)[*current_line_size - 1] = CHAR_ENDOFSTRING;
            file_streamer->callback(*line_buffer,context);
            *current_line_size = 0;
        }
    }
    return 1;
}

int file_streamer_run(file_streamer_t* file_streamer,void* context){
    char file_buffer[FILE_BUFFER_SIZE] = "";
    char* line_buffer = malloc(sizeof(char)*LINE_BUFFER_INITIAL_SIZE);
    if ( !line_buffer ) return -1; /*Si falla el malloc*/

    size_t line_buffer_size = LINE_BUFFER_INITIAL_SIZE;
    size_t current_line_size = 0;

    while ( !feof(file_streamer->file) ){
        size_t bytes_read = fread(file_buffer,1,FILE_BUFFER_SIZE,
                            file_streamer->file);
        if ( fill_line_buffer(file_streamer,context,bytes_read,&line_buffer,
                        file_buffer,&current_line_size,&line_buffer_size) < 0){
                            free(line_buffer);
                            return -1;
                        }
    }
    free(line_buffer);
    return 0;
}

void file_streamer_destroy(file_streamer_t* file_streamer){}
