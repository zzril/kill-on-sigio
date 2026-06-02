NAME=kill-on-sigio

SRC_DIR=./src
BLD_DIR=./build
BIN_DIR=./bin

SOURCES=$(wildcard $(SRC_DIR)/*.c)

OBJECTS=$(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(SOURCES))

BIN=$(BIN_DIR)/$(NAME)

# --------

FLAGS=-Wall -Wextra -pedantic -g

CC=clang
CFLAGS=$(FLAGS)

LD=$(CC)
LDFLAGS=$(FLAGS)

# --------

.PHONY: all
all: $(BIN)

.PHONY: clean
clean:
	rm -r $(BLD_DIR) $(BIN_DIR) || true

# --------

$(BIN): $(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(LD) $(LDFLAGS) -o $@ $^

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BLD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

