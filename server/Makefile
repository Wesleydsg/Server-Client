TARGET = server

CC = gcc

CFLAGS = -Wall -Wextra -O2

SRC = http_server.c

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

clean:
	rm -f $(TARGET)