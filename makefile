VERSION = "0.0.1"

CC = cc
CCFLAGS = -Wall -Wextra -Wpedantic -Os

PREFIX ?= /usr/local
DESTDIR ?= /bin

SRC = ./src
BIN = ./bin
SRCS = $(wildcard $(SRC)/*.c)
BINS = $(patsubst $(SRC)/%.c,$(BIN)/%,$(SRCS))

.PHONY: all clean install

all: $(BINS)
$(BIN)/%: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -DVERSION='$(VERSION)' -o $@ $<

clean:
	rm -fr $(BIN)

install: $(BINS)
	@printf "\x1b[4;32mTODO: Make instalation work.\x1b[0m\n"

