# Variables
CC = gcc
CFLAGS = -Wall -g -Wextra -I./src/discordSDK
LIBS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -ljansson -L./src/discordSDK/lib/x86_64 -ldiscord_game_sdk -Wl,-rpath=./src/discordSDK/lib/x86_64
BUILD_DIR = build

# Windows-specific variables
CC_WIN = x86_64-w64-mingw32-gcc
LIBS_WIN = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -ljansson -L./src/discordSDK/lib/x86_64 -ldiscord_game_sdk

# Main target (default: Linux)
all: dirs linux

# Create build directories
dirs:
	mkdir -p $(BUILD_DIR)

# Compile game executable for Linux
linux: $(BUILD_DIR)/game_linux

$(BUILD_DIR)/game_linux: $(BUILD_DIR)/game.o $(BUILD_DIR)/engine.o $(BUILD_DIR)/audio.o $(BUILD_DIR)/graphics.o $(BUILD_DIR)/discord.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

# Compile game executable for Windows
windows: CC = $(CC_WIN)
windows: LIBS = $(LIBS_WIN)
windows: dirs $(BUILD_DIR)/game_windows.exe

$(BUILD_DIR)/game_windows.exe: $(BUILD_DIR)/game.o $(BUILD_DIR)/engine.o $(BUILD_DIR)/audio.o $(BUILD_DIR)/graphics.o $(BUILD_DIR)/discord.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

# Compile object files
$(BUILD_DIR)/game.o: src/game.c src/game.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/discord.o: src/discord.c src/discord.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/engine.o: src/engine/engine.c src/engine/engine.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/audio.o: src/engine/audio.c src/engine/audio.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/graphics.o: src/engine/graphics.c src/engine/graphics.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build directory
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all dirs clean linux windows

#TODO ZLIB DLL ERRORS AND ALSO SHOULD I INCLUDE DLLS / CHANGE WINDOWS BUILD STEPS