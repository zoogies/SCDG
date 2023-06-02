#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <SDL2/SDL_ttf.h>

// enum denoting all possible renderObject types
typedef enum {
    renderType_Text,
    renderType_Image,
    renderType_Button,
} renderObjectType;

// struct defining renderObject(s)
typedef struct renderObject {
    // common to every render object
    int identifier;
    int depth;
    renderObjectType type;
    SDL_Texture* texture;
    SDL_Rect rect;
    struct renderObject* next;
} renderObject;

// linked list holding pointers towards button render objects
typedef struct button {
    struct renderObject *object;
    struct button *next;
} button;

void addRenderObject(int identifier, renderObjectType type, int depth, float x, float y, float width, float height, SDL_Texture *texture, bool centered);

void removeRenderObject(int identifier);

renderObject* getRenderObject(int identifier);

TTF_Font* loadFont(const char* fontPath, int fontSize);

SDL_Texture* createTextTexture(const char* text, TTF_Font* font, SDL_Color *color);

int createText(int depth, float x,float y, float width, float height, char *text, TTF_Font *font, SDL_Color *color, bool centered);

int createImage(int depth, float x, float y, float width, float height, char *path, bool centered);

void renderAll();

void initGraphics(int screenWidth,int screenHeight, int windowMode, int framecap);

void shutdownGraphics();

#endif