#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "graphics.h"

/*
    Reserve an internal font and color for the engine to use rendering
    overlays such as the fpsCounter. Font NULL until initialized in init()
*/
extern SDL_Color *pEngineFontColor;
extern TTF_Font *pEngineFont;

// entry point to the engine, initializes all subsystems
void initEngine(int screenWidth, int screenHeight, bool debug, int volume, int windowMode, int framecap, bool skipintro);

// function to print out (complete) with "complete" displayed in green color
void debugOutputComplete();

// shutdown point for engine, shuts down all subsystems
void shutdownEngine();

#endif