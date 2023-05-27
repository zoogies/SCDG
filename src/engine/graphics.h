#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <SDL2/SDL_ttf.h>

typedef enum {
    renderType_Text,
    renderType_Image,
} renderObjectType;

typedef struct renderObject {
    // common to every render object
    int identifier;
    renderObjectType type;
    SDL_Texture* texture;
    SDL_Rect rect;
    int depth;
    struct renderObject* next;

    // text specific fields
    TTF_Font* font;
    SDL_Color color;
} renderObject;

// TODO: consider having images in engine start at their center, so game has to do no calculations for finding middles of things
void addRenderObject(int identifier, renderObjectType type, int depth, int x, int y, int width, int height, SDL_Texture *texture, TTF_Font* font, SDL_Color color);

void removeRenderObject(int identifier);

renderObject* getRenderObject(int identifier);

// load a font into memory and return a pointer to it
TTF_Font* loadFont(const char* fontPath, int fontSize);

int renderText(int depth, int x,int y, int width, int height, char *text, TTF_Font *font, SDL_Color color);

int renderImage(int depth, int x, int y, int width, int height, char *path);

void renderAll();

// initialize graphics
void initGraphics(int screenWidth,int screenHeight);

void shutdownGraphics();

#endif