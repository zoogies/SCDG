# Variables
CC = gcc
CFLAGS = -Wall -g
LIBS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
BUILD_DIR = build

# Main target
all: dirs $(BUILD_DIR)/game

# Create build directories
dirs:
	mkdir -p $(BUILD_DIR)

# Compile game executable
$(BUILD_DIR)/game: $(BUILD_DIR)/game.o $(BUILD_DIR)/engine.o $(BUILD_DIR)/audio.o $(BUILD_DIR)/graphics.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

# Compile object files
$(BUILD_DIR)/%.o: src/engine/%.c src/engine/%.h src/game.c src/game.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.c src/%.h src/engine/engine.c src/engine/engine.h src/engine/audio.c src/engine/audio.h src/engine/graphics.c src/engine/graphics.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build directory
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all dirs clean