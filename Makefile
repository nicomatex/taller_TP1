CC = gcc
CFLAGS = -g -std=c99 -Wtype-limits -pedantic -Wconversion -Wno-sign-conversion -Wall
VFLAGS = --leak-check=full --track-origins=yes --show-reachable=yes

CCLIENT = cliente_main.c
CSERVER = server_main.c
CTEST = test.c
CTDA = command_parser.c command_serializator.c file_streamer.c network_util.c client.c server.c
HTDA = command_parser.h command_serializator.h file_streamer.h network_util.h client.h server.h
EXEC_TEST = pruebas
EXEC_SERVER = server
EXEC_CLIENT = cliente


test_build: $(CTDA)
	$(CC) $(CFLAGS) -o $(EXEC_TEST) $(CTEST) $(CTDA)

test_run: test_build
	./$(EXEC_TEST)

test_valgrind: test_build
	valgrind $(VFLAGS) ./$(EXEC_TEST)

client_build: $(CTDA)
	$(CC) $(CFLAGS) -o $(EXEC_CLIENT) $(CCLIENT) $(CTDA)

client_run: client_build
	./$(EXEC_CLIENT)

client_valgrind: client_build
	valgrind $(VFLAGS) ./$(EXEC_CLIENT)

server_build: $(CTDA)
	$(CC) $(CFLAGS) -o $(EXEC_SERVER) $(CSERVER) $(CTDA)

server_run: server_build
	./$(EXEC_SERVER)

server_valgrind: server_build
	valgrind $(VFLAGS) ./$(SERVER_CLIENT)

clean:
	rm -f *.o $(EXEC_TEST) $(EXEC_SERVER) $(EXEC_CLIENT)
