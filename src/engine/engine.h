#ifndef ENGINE_H
#define ENGINE_H

// entry point to the engine, initializes all subsystems
void initEngine(int screenWidth, int screenHeight);

// function to print out (complete) with "complete" displayed in green color
void debugOutputComplete();

// shutdown point for engine, shuts down all subsystems
void shutdownEngine();

#endif