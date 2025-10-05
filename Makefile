CC=gcc
CFLAGS=-Wall -Wextra -pthread
OBJ_SERVER=server.o
OBJ_CLIENT=client.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: server client

server: $(OBJ_SERVER)
	$(CC) -o $@ $^ $(CFLAGS)

client: $(OBJ_CLIENT)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f server client *.o

.PHONY: all clean