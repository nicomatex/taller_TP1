#ifndef __CLIENT_DBUS_H
#define __CLIENT_DBUS_H

/* Funcion de callback que recibe una linea y un cliente a traves de 
context, y envia a traves del cliente el comando DBUS representado 
por la linea.*/
void dbus_parse_and_send(char* line,void* context);

#endif
