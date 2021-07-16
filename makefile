CC = g++
CC_ARGS = -std=c++11

all: server client router

server: server.cpp
	$(CC) $(CC_ARGS) server.cpp -o server

router: router.cpp
	$(CC) $(CC_ARGS) router.cpp -o router

client: client.cpp
	$(CC) $(CC_ARGS) client.cpp -o client

clean: saaf
	rm -rf *.o *.out server router client

saaf:
	rm -f fifo_* file_*