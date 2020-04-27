CC = gcc
CFLAGS = -g -std=c99 -Wtype-limits -pedantic -Wconversion -Wno-sign-conversion -Wall
VFLAGS = --leak-check=full --track-origins=yes --show-reachable=yes

CCLIENT = cliente_main.c
CSERVER = server_main.c
CTEST = test.c
CTDA = command_parser.c command_serializator.c file_streamer.c network_util.c
HTDA = command_parser.h command_serializator.h file_streamer.h network_util.h
EXEC_TEST = pruebas
EXEC_SERVER = server
EXEC_CLIENT = cliente


build_test: $(CTDA)
	$(CC) $(CFLAGS) -o $(EXEC_TEST) $(CTEST) $(CTDA)

run_test: build_test
	./$(EXEC_TEST)

valgrind_test: build_test
	valgrind $(VFLAGS) ./$(EXEC_TEST)

clean:
	rm -f *.o $(EXEC_S)
