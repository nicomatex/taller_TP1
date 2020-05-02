#ifndef CLIENT_H
#define CLIENT_H
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct client{
    int skt;
    char* host;
    char* port;
    uint32_t current_msg_id;
}client_t;

/*Inicializa el cliente pasado por parametro, con el host y el port indicados.*/
void client_create(client_t* client,char* host,char* port);

/*Intenta conectar el cliente al servidor.
Devuelve True si la conexion fue exitosa, False si no.*/
int client_connect(client_t* client);

/*Intenta enviar un mensaje message de tamanioo size a traves del cliente.
Devuelve la cantidad de bytes enviados si el envio fue exitoso, -1 en caso de error.*/
int client_send_msg(client_t* client,unsigned char* message,size_t size);

/*Intenta recibir un mensaje de tamanio recieve_size a traves del cliente, y lo almacena
en bufer. Devuelve la cantidad de bytes recibidos en caso exitoso, y -1 en caso de error.*/
int client_recv_msg(client_t* client,unsigned char* buffer,size_t recieve_size);

/*Desconecta al cliente.El cliente debe haber sido conectado anteriormente.*/
void client_disconnect(client_t* client);

/*Destruye un cliente.*/
void client_destroy(client_t* client);
#endif
