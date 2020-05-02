# Taller de Programacion I [75.42] - TP1

**Nombre: Nicolas Federico Aguerre**

**Padron: 102145**

**Repositorio: https://github.com/nicomatex/taller_TP1**

**Cuatrimestre: 1C 2020**

---

# Indice del informe
1. **[Cliente](#Cliente)**
    * 1.1. [File Streamer](#Cliente_1)
    * 1.2. [Command Parser](#Cliente_2)
    * 1.3. [Command Serializator](#Cliente_3)
    * 1.4. [Client](#Cliente_4)

2. **[Servidor](#Servidor)**

    * 2.1. [Server](#Servidor_1)
    * 2.2. [Command Deserializator](#Servidor_2)
---

## Cliente <a name="Cliente"></a>

El cliente propuesto a desarrollar por la consigna de este trabajo práctico
debía ser capaz de leer de algún archivo (ya fuera un archivo de texto o
la propia entrada estándar), parsear los comandos representados en las líneas
de dicho archivo, generar a partir de este comando un mensaje siguiendo el 
protocolo DBUS, enviar dicho mensaje al servidor, y por ultimo mostrar la 
respuesta del servidor a dicho mensaje.

Debido a que el funcionamiento de este cliente está compuesto por tareas que,
en principio, no tienen nada que ver entre sí, se pensó el cliente de forma 
modular, donde cada módulo cumple una de las tareas específicadas arriba, y 
el cliente en su totalidad no es mas que cada módulo cumpliendo su tarea en 
un orden específico, como si de una "pipeline" se tratara.

En concreto, los módulos en los cuales se divide el cliente son los siguientes:

1. File Streamer
2. Command Parser
3. Command Serializator
4. Client (network)

En resumidas palabras, el flujo de funcionamiento del programa es:

1. El file streamer lee del archivo una línea, y se la pasa al command parser
2. El Command Parser interpreta esta línea, y acomoda cada parámetro en una
estructura de formato conocido.
3. Esta estructura le es pasada al Command Serializator, que genera a partir de 
la informacion almacenada en dicha estructura un mensaje con el formato DBUS 
(es decir, una secuencia de bytes en un formato especifico).
4. Finalmente, esta secuencia de bytes le es pasada al cliente, y el cliente 
se encarga de transmitirsela al servidor, esperar su respuesta, 
y mostrarla por pantalla.

![Imagen 1](img/1.png?raw=true)

_Imagen 1.Diagrama en bloques del cliente_

A continuacion se detallan los aspectos mas importantes de la implementacion
de cada modulo.

---

**1.1. File Streamer**<a name="Cliente_1"></a>

El streamer de archivos opera con dos buffers. Un buffer estático de 32 bytes 
de tamaño, y un buffer dinámico. El buffer estático se utiliza para cargar 
32 bytes del archivo a la memoria (de acuerdo a lo pedido por el enunciado).
Una vez cargados estos 32 bytes, se empiezan a copiar 1 a 1 al buffer dinámico.
Si en este proceso de copiado llegase a aparecer un salto de línea, entonces 
se dispara la secuencia indicada en la Imagen 1 mediante una llamada
a la funcion de callback con el contenido del buffer dinámico, que será de
una línea del archivo. Si se llega al final 
del buffer estático y no se encontró un salto de línea, entonces 
se leen 32 bytes más, y se repite el proceso, con la diferencia de que
el contenido se va "concatenando" en el buffer dinámico, lo cual quiere
decir que el buffer dinámico siempre se llena hasta completar una línea.
Si en algún momento de este proceso se llena el buffer dinámico, simplemente 
se reubica en la memoria con un tamaño mayor. Este proceso
continúa hasta agotar el archivo.

![Imagen 2](img/2.png?raw=true)

_Imagen 2.Operacion de la carga del archivo en buffers_

---

**1.2. Command Parser**<a name="Cliente_2"></a>

El Command Parser define un struct command_t que contiene un campo por cada uno de los 
parametros que contiene un comando DBUS (path, interfaz, destino, metodo y 
signature parameters). Dado que los signature parameters(aquellos que 
dependen del metodo) se almacenan como un
string con los parametros separados por coma, para comodidad en el uso de 
otras secciones el programa, tambien se incluye un campo **signature_parameter_count**,
que indica la cantidad de signature parameters del metodo.

Este modulo toma una linea en forma de array de caracteres terminado en NULL, y le 
hace dos pasadas: En la primera pasada, se determina cual es el tamaño necesario
para almacenar cada uno de los parametros. Luego, se reserva la memoria necesaria
para guardar dichos parametros, y finalmente se hace una segunda pasada en la cual 
se copia cada parámetro a su respectivo campo del struct command_t, utilizando
los separadores adecuados dentro de la linea.

---

**1.3. Command Serializator**<a name="Cliente_3"></a>

Este módulo genera, a partir de un struct command_t, el arreglo de 
bytes correspondiente a dicho comando en el formato especificado por el 
protocolo DBUS. 

En primer lugar, se genera el header del mensaje. Para ello, se 
calcula el tamaño necesario para almacenar la totalidad del header. Este 
tamaño consiste de un cierto overhead necesario para almacenar 
la información obligatoria que contendrá el mensaje, y luego de un array 
en el cual estará almacenada la información de cada uno de los parámetros de nuestro mensaje.
Cada uno de los elementos de este array a su vez tienen un overhead que consiste de 
los datos del parámetro (es decir, su tamaño, tipo de dato, y tipo de parámetro), 
y el dato en cuestión junto con el correspondiente padding.

Una vez calculado el tamaño del header, se reserva la memoria necesaria, y se procede
a ubicar cada uno de los elementos del mismo en la posición correspondiente, mediante
el uso de un puntero que sirve como "posicion de escritura" y una serie de funciones
que escriben en una posicion determinada un cierto elemento del array de parámetros.

Luego, si el mensaje contiene signature parameters, entonces se repite el proceso
con el cuerpo del mensaje. 

Finalmente, se concatenan el header y el cuerpo del mensaje en un solo mensaje, y 
ese mensaje es la salida del modulo. 

---

**1.4. Client**<a name="Cliente_4"></a>

El cliente es un módulo que es completamente ajeno a la lógica de parseo y generación
del mensaje a enviar. Lo único que es capaz de hacer es tomar un arreglo de bytes y 
enviárselo al servidor indicado. 

Para lograr esto, el cliente solicita primero la información DNS correspondientes 
al puerto y host indicados por el usuario, mediante getaddrinfo. Luego, se conecta
al primer resultado que le sea posible de esta solicitud. 

Una vez está en estado conectado, ya está en condiciones de enviar mensajes 
mediante la escritura en el File Descriptor indicado por el socket. Será
a través de este socket que enviará el mensaje con formato DBUS generado
por los módulos anteriores. 

Una vez enviado el mensaje, el cliente queda a la espera de la respuesta del servidor, y, una
vez obtenida dicha respuesta, el flujo del programa empieza nuevamente desde el principio
con el siguiente comando a procesar. 

---

## Servidor <a name="Servidor"></a>

En contrapunto al cliente, lo requerido por parte de la consigna para este servidor es 
ser capaz de recibir un array de bytes que contenian un comando formateado según el protocolo
DBUS, deserializar dicho comando, e imprimir por pantalla cada uno de sus parámetros.

Al igual que en el cliente, debido a la diversidad de tareas no relacionadas 
que debía cumplir el servidor, se utilizó una lógica modular dividida de la siguiente manera:

1. Servidor
2. Command Deserializer

En resumidas palabras, el flujo de funcionamiento del servidor sería el siguiente:

1. Se levanta el servidor, a la escucha de nuevas conexiones. 
2. Cuando hay una conexión entrante (de un cliente), se acepta dicha conexión. 
3. Se recibe el mensaje, y se lo decodifica. De ser exitoso este proceso, se 
envia al cliente un mensaje "OK".
4. Se vuelve al paso 3, y este proceso se repite hasta que el cliente cierre la conexion.
5. Cuando el cliente cierra la conexión, se desconecta el servidor y el programa termina.

![Imagen 3](img/3.png?raw=true)

_Imagen 3.Diagrama de flujo del servidor_

---

**2.1. Server**<a name="Server_1"></a>

El modulo server en si, es, al igual que el cliente, completamente ajeno al protocolo DBUS, y
lo único que sabe hacer es escuchar una conexión de un cliente, e intercambiar mensajes 
con dicho cliente. Para ello, primero se establece el servidor en modo escucha en el puerto indicado
por el cliente creando un socket acceptor. Cuando llega una conexión, se acepta mediante dicho socket
acceptor, y se crea un nuevo socket que representa el canal de comunicación con el cliente. Se reciben
entonces una cantidad de bytes correspondientes al tamaño básico del header DBUS, que contiene toda la 
información que necesitamos para recibir el resto del mensaje, esto es, el tamaño del array de parámetros
y el tamaño del cuerpo. 

Una vez obtenidos los tamaños del array de parámetros y del cuerpo, se procede a recibir el resto del mensaje (cuyo tamaño
restante es la suma de dichas dos cosas).

---

**2.2. Command Deserializator**<a name="Server_2"></a>

La funcionalidad de este módulo es, dado un arreglo de bytes, deserializar el mensaje en formato 
protocolo DBUS representado en dicho arreglo, y rellenar los campos de un struct command_t, que luego
será utilizado para imprimir en pantalla los parámetros del comando.

Para realizar esto, se utilizan principalmente dos funciones auxiliares, cada una de las cuales
es capaz de decodificar un tipo de valor del mensaje. La primera de ellas es para decodificar enteros de tipo
uint32_t en una determinada posición del array, interpretando los 4 bytes correspondientes (que deben estar
formateados en little endian). Y la segunda, que es capaz de leer un string a partir de la posición indicada hasta el 
final de string (\0).