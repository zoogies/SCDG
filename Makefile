# Variables
CC = gcc
CFLAGS = -Wall -g -Wextra -I./src/discordSDK
LIBS = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -ljansson -L./src/discordSDK/lib/x86_64 -ldiscord_game_sdk -Wl,-rpath=./src/discordSDK/lib/x86_64
BUILD_DIR_LINUX = build/linux
BUILD_DIR_WINDOWS = build/windows

# Windows-specific variables
CC_WIN = x86_64-w64-mingw32-gcc
LIBS_WIN = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -ljansson -L./src/discordSDK/lib/x86_64 -ldiscord_game_sdk

# Main target (default: Linux)
all: dirs linux

# Create build directories
dirs:
	mkdir -p $(BUILD_DIR_LINUX) $(BUILD_DIR_WINDOWS)

# Compile game executable for Linux
linux: $(BUILD_DIR_LINUX)/game_linux

$(BUILD_DIR_LINUX)/game_linux: $(BUILD_DIR_LINUX)/game.o $(BUILD_DIR_LINUX)/engine.o $(BUILD_DIR_LINUX)/audio.o $(BUILD_DIR_LINUX)/graphics.o $(BUILD_DIR_LINUX)/discord.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@
	rm -f $(BUILD_DIR_LINUX)/*.o

# Compile game executable for Windows
windows: CC = $(CC_WIN)
windows: LIBS = $(LIBS_WIN)
windows: dirs $(BUILD_DIR_WINDOWS)/game_windows.exe copy_resources_windows

$(BUILD_DIR_WINDOWS)/game_windows.exe: $(BUILD_DIR_WINDOWS)/game.o $(BUILD_DIR_WINDOWS)/engine.o $(BUILD_DIR_WINDOWS)/audio.o $(BUILD_DIR_WINDOWS)/graphics.o $(BUILD_DIR_WINDOWS)/discord.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@
	rm -f $(BUILD_DIR_WINDOWS)/*.o

# Compile object files
$(BUILD_DIR_LINUX)/%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_LINUX)/%.o: src/engine/%.c src/engine/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_WINDOWS)/%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_WINDOWS)/%.o: src/engine/%.c src/engine/%.h
	$(CC) $(CFLAGS) -c $< -o $@

# Copy resources/dlls to Windows build directory
copy_resources_windows: 
	cp -r resources/dlls/* $(BUILD_DIR_WINDOWS)/

# Clean up build directories
clean:
	rm -rf $(BUILD_DIR_LINUX) $(BUILD_DIR_WINDOWS)

.PHONY: all dirs clean linux windows copy_resources_windows