#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>

typedef struct server{
    int skt;
    int client_skt;
    char* port;
}server_t;

/*Recibe un server_t y un puerto, y almacena los datos necesarios en server para 
establecer un servidor que escuche conexiones en ese puerto.*/
void server_create(server_t* server,char* port);

/*Establece la conexion del servidor y lo pone a la escucha de nuevas conexiones. Devuelve
-1 en caso de error.*/
int server_connect(server_t* server);

/*Acepta una conexion entrante al servidor. Devuelve -1 en caso de error.*/
int server_accept_conection(server_t* server);

/*Envia un mensaje a traves de la conexion activa del servidor.
Devuelve la cantidad de bytes enviados, 0 si la conexion fue cerrada desde
el otro extremo, y -1 en caso de error.*/
int server_send_msg(server_t* server,unsigned char* message,size_t msg_size);

/*Recibe un mensaje a traves de la conexion activa del servidor. Devuelve la cantidad 
de bytes recibidos, 0 si la conexion fue cerrada desde l otro extremo, y -1 en caso de error.*/
int server_recv_msg(server_t* server,unsigned char* buffer,size_t recieve_size);

/*Cierra la conexion actual del servidor y deja de escuchar conexiones nuevas.*/
void server_disconnect(server_t* server);

/*Destruye el servidor.*/
void server_destroy(server_t* server);
#endif
