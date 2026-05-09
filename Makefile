CC = gcc

CFLAGS = -Wall -Wextra -std=c11 $(shell pkg-config --cflags notcurses-core)
LIBS = $(shell pkg-config --libs notcurses-core)

TARGET = minesweeper
SRC = minesweeper.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LIBS)

clean: 
	rm -f $(TARGET)