
CC = gcc

SERVER_SRC = server.c game.c
CLIENT_SRC = client.c

SERVER_HEADERS = server.h game.h
CLIENT_HEADERS = client.h 

SERVER_OUT = server
CLIENT_OUT = client

all: $(SERVER_OUT) $(CLIENT_OUT)

$(SERVER_OUT): $(SERVER_SRC) $(SERVER_HEADERS)
	$(CC)  -o $(SERVER_OUT) $(SERVER_SRC)

$(CLIENT_OUT): $(CLIENT_SRC) $(CLIENT_HEADERS)
	$(CC)  -o $(CLIENT_OUT) $(CLIENT_SRC)

clean:
	rm -f $(SERVER_OUT) $(CLIENT_OUT)

.PHONY: all clean