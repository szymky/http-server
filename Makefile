

TARGET := http-server

CC := gcc
CFLAGS := -Wall -Wextra


.PHONY: all run

all:
	$(CC) $(CFLAGS) -o $(TARGET) src/main.c 

run: all 
	./$(TARGET)
