CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = tarsau

all: $(TARGET)

$(TARGET): main.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o -lm

main.o: main.c functions.h
	$(CC) $(CFLAGS) -c main.c

clean:
	rm -f $(TARGET) *.o

.PHONY: all clean

