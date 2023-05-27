#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>

// entry point to the engine, initializes all subsystems
void initEngine(int screenWidth, int screenHeight, bool debug);

// function to print out (complete) with "complete" displayed in green color
void debugOutputComplete();

// shutdown point for engine, shuts down all subsystems
void shutdownEngine();

#endif