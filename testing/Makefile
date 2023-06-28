CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -ljansson

TARGET = jansson

all: $(TARGET)

$(TARGET): jansson.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
