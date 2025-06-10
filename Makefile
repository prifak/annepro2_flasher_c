CC = gcc
CFLAGS = -Wall -Wextra -O2 `pkg-config --cflags hidapi-hidraw`
LDFLAGS = `pkg-config --libs hidapi-hidraw`

SRC_DIR = src
BUILD_DIR = .build
BIN_DIR = bin
OUT = $(BIN_DIR)/annepro2_flasher_c

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

all: $(BIN_DIR) $(BUILD_DIR) $(OUT)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR) $(BUILD_DIR):
	mkdir -p $@

install: $(OUT)
	install -Dm755 $(OUT) $(BINDIR)/annepro2_tools_c

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
