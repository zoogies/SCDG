#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <SDL2/SDL_ttf.h>

// function prototypes and structure declarations
void initGraphics(int screenWidth,int screenHeight);

TTF_Font* loadFont(const char* fontPath, int fontSize);

void renderText(SDL_Renderer* renderer, const char* text, TTF_Font* font, SDL_Color color, SDL_Rect* rect);

// shutdown all graphics elements and free all assosciated memory
void shutdownGraphics();

#endif