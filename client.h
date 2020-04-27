#ifndef CLIENT_H
#define CLIENT_H
#include <stdbool.h>

typedef struct client{
    int skt;
    char* host;
    char* port;
}client_t;

/*Inicializa el cliente pasado por parametro, con el host y el port indicados.*/
void client_create(client_t* client,char* host,char* port);

/*Intenta conectar el cliente al servidor.
Devuelve True si la conexion fue exitosa, False si no.*/
int client_connect(client_t* client);

/*Intenta enviar un mensaje a traves del cliente.
Devuelve true si el envio fue exitoso, false en caso contrario.*/
int client_send_msg(client_t* client,unsigned char* message,size_t size);

/*Desconecta al cliente.El cliente debe haber sido conectado anteriormente.*/
void client_disconnect(client_t* client);

/*Destruye un cliente.*/
void client_destroy(client_t* client);
#endif