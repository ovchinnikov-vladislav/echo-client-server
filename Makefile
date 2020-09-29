BUILD_DIR=build
BIN_DIR=$(BUILD_DIR)/bin
SRC_DIR=.

CC=gcc

SERVER_O=$(BUILD_DIR)/server.o
CLIENT_O=$(BUILD_DIR)/client.o

SERVER_C=$(SRC_DIR)/server.c
CLIENT_C=$(SRC_DIR)/client.c

clear:
	rm -R $(BUILD_DIR) -f
	rm -R $(BIN_DIR) -f

mkd: clear
	mkdir $(BUILD_DIR)
	mkdir $(BIN_DIR)

compile_server: mkd
	$(CC) $(SERVER_C) -c -o $(SERVER_O)

compile_client: mkd
	$(CC) $(CLIENT_C) -c -o $(CLIENT_O)

compile_all: compile_server compile_client

link_server: compile_server
	$(CC) -o $(BIN_DIR)/server.bin $(SERVER_O)

link_client: compile_client
	$(CC) -o $(BIN_DIR)/client.bin $(CLIENT_O)

link_all: link_server link_client