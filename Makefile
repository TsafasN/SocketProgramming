CC=gcc
CFLAGS=-Wall -Wextra -pthread
INCLUDES=-I.

# Directories
BUILD_DIR=build
BIN_DIR=$(BUILD_DIR)/bin
OBJ_DIR=$(BUILD_DIR)/obj

# Source files
SRC_DIR=.
UTIL_DIR=util

# Object files
UTIL_OBJS=$(OBJ_DIR)/$(UTIL_DIR)/socketUtil.o
SERVER_OBJS=$(OBJ_DIR)/server.o $(UTIL_OBJS)
CLIENT_OBJS=$(OBJ_DIR)/client.o $(UTIL_OBJS)

# Final binaries
SERVER_BIN=$(BIN_DIR)/server
CLIENT_BIN=$(BIN_DIR)/client

# Create necessary directories
$(shell mkdir -p $(BIN_DIR) $(OBJ_DIR)/$(UTIL_DIR))

# Build rules
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

all: $(SERVER_BIN) $(CLIENT_BIN)

$(SERVER_BIN): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(CLIENT_BIN): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD_DIR)

run_server: $(SERVER_BIN)
	$(SERVER_BIN)

run_client: $(CLIENT_BIN)
	$(CLIENT_BIN)

.PHONY: all clean run_server run_client

.PHONY: all clean