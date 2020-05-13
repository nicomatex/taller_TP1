#ifndef __SERVER_DBUS_H
#define __SERVER_DBUS_H

#include "server.h"

/* Establece un servidor apto para recibir mensajes en protoclo
DBUS de un cliente en el servidor pasado por parametro.*/
void dbus_server_run(server_t* server);

#endif
