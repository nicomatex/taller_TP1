#ifndef NETWORK_UTIL_H
#define NETWORK_UTIL_H
#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdbool.h>

/*Recibe una direccion ip o dominio de un host, un puerto, un struct addrinfo en el cual guardar 
resultados, y un booleano is_server, y obtiene la informacion necesaria para la conexion
a dicho host en dicho puerto y lo almacena en result. Si se trata de un server, se 
debe aclarar is_server = true. 
Devuelve true si la obtencion de datos fue exitosa, false en caso
contrario.*/
bool get_info_from_dns(char* host, char* port, struct addrinfo** result, bool is_server);

/*Recibe un puntero a un socket, y un struct addrinfo con informacion DNS. Intenta 
establecer una conexion con el servidor. De ser exitoso, se establece el socket pasado
por parametro en modo CONNECTED. 
Devuelve True si la conexion fue exitosa, false en caso contrario.*/
bool connect_to_available_server(int* skt, struct addrinfo* results);

/*Recibe un socket en estado CONNECTED skt, un buffer de salida output_buffer, y una cantidad
de bytes size. Intenta enviar a traves de skt  size bytes de output_buffer. Devuelve 
la cantidad de bytes enviados en caso exitoso, y -1 en caso de error.*/
int send_message(int skt,unsigned char *output_buffer, size_t size);

/*Recibe un socket en estado CONNECTED skt, un buffer cual almacenar un mensaje, y una cantidad
size de bytes a leer. Intenta leer del servidor. Devuelve la cantidad de bytes recibidos
en caso exitoso, y -1 en caso de error.*/
int recieve_message(int skt,unsigned char* buffer,size_t recieve_size);
#endif