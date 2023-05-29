#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>

// declare global helper const for tracking middle coords of screen
extern int SCREEN_MIDDLE_WIDTH;
extern int SCREEN_MIDDLE_HEIGHT;

// entry point to the engine, initializes all subsystems
void initEngine(int screenWidth, int screenHeight, bool debug, int volume, int windowMode, int framecap);

// function to print out (complete) with "complete" displayed in green color
void debugOutputComplete();

// shutdown point for engine, shuts down all subsystems
void shutdownEngine();

#endif