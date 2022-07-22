
# --------

NAME=helloworld
MAIN=main
LIB_NAME=libevil

SRC_DIR=src
BIN_DIR=bin
LIB_DIR=lib

LIB_SRC_DIR=$(LIB_DIR)/src
LIB_BIN_DIR=$(LIB_DIR)/bin

SRC=$(SRC_DIR)/$(MAIN).c
BIN=$(BIN_DIR)/$(NAME)

LIB_SRC=$(LIB_SRC_DIR)/$(LIB_NAME).c
LIB_BIN=$(LIB_BIN_DIR)/$(LIB_NAME).so

CC=clang
CFLAGS=-Wall -Wextra -pedantic
LIB_CFLAGS=$(CFLAGS) -shared -fPIC
LIB_LD_FLAGS=-ldl

# --------

all: $(BIN)

lib: $(LIB_BIN)

# --------

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

$(LIB_BIN): $(LIB_SRC)
	$(CC) $(LIB_CFLAGS) -o $@ $^ $(LIB_LD_FLAGS)

# --------


