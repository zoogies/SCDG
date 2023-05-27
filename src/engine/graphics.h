#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <SDL2/SDL_ttf.h>

typedef struct renderObject {
    int identifier;
    // renderObjectType type;
    SDL_Texture* texture;
    SDL_Rect rect;
    int depth;
    struct renderObject* next;
} renderObject;

// TODO: consider having images in engine start at their center, so game has to do no calculations for finding middles of things
void addRenderObject(int identifier, int depth, int x, int y, int width, int height, SDL_Texture *texture);

void removeRenderObject(int identifier);

renderObject* getRenderObject(int identifier);

void renderAll();

// initialize graphics
void initGraphics(int screenWidth,int screenHeight);

void shutdownGraphics();

// load a font into memory and return a pointer to it
TTF_Font* loadFont(const char* fontPath, int fontSize);

SDL_Texture* createTextTexture(const char* text, TTF_Font* font, SDL_Color color);

// Create a texture from image path
SDL_Texture* createImageTexture(const char* path);

#endif