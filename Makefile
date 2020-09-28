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

build_server: mkd
	$(CC) $(SERVER_C) -o $(SERVER_O)

build_client: mkd
	$(CC) $(CLIENT_C) -o $(CLIENT_O)

build_all: build_server build_client