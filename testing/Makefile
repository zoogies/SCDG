CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -ljansson

TARGET = jansson variant

all: $(TARGET)

$(TARGET): jansson.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

$(TARGET): variant.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET)
