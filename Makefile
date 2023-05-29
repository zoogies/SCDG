# Variables
CC = gcc
CFLAGS = -Wall -g
LIBS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -ljansson
BUILD_DIR = build
BUILD_DIR_WIN = build-win
CC_WIN = x86_64-w64-mingw32-gcc
EXE_WIN = game.exe

# Main target
all: dirs $(BUILD_DIR)/game

# Create build directories
dirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR_WIN)

# Compile game executable
$(BUILD_DIR)/game: $(BUILD_DIR)/game.o $(BUILD_DIR)/engine.o $(BUILD_DIR)/audio.o $(BUILD_DIR)/graphics.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

# Compile Windows game executable
windows: dirs $(BUILD_DIR_WIN)/$(EXE_WIN)

$(BUILD_DIR_WIN)/$(EXE_WIN): $(BUILD_DIR_WIN)/game.o $(BUILD_DIR_WIN)/engine.o $(BUILD_DIR_WIN)/audio.o $(BUILD_DIR_WIN)/graphics.o
	$(CC_WIN) $(CFLAGS) $^ $(LIBS) -o $@

# Compile object files
$(BUILD_DIR)/%.o: src/engine/%.c src/engine/%.h src/game.c src/game.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_WIN)/%.o: src/engine/%.c src/engine/%.h src/game.c src/game.h
	$(CC_WIN) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.c src/%.h src/engine/engine.c src/engine/engine.h src/engine/audio.c src/engine/audio.h src/engine/graphics.c src/engine/graphics.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_WIN)/%.o: src/%.c src/%.h src/engine/engine.c src/engine/engine.h src/engine/audio.c src/engine/audio.h src/engine/graphics.c src/engine/graphics.h
	$(CC_WIN) $(CFLAGS) -c $< -o $@

# Clean up build directory
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BUILD_DIR_WIN)

.PHONY: all dirs windows clean