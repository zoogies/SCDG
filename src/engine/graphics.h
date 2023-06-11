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
    SDL_Texture *pTexture;
    SDL_Rect rect;
    struct renderObject *pNext;
} renderObject;

// linked list holding pointers towards button render objects
typedef struct button {
    struct renderObject *pObject;
    struct button *pNext;
} button;

void addRenderObject(int identifier, renderObjectType type, int depth, float x, float y, float width, float height, SDL_Texture *pTexture, bool centered);

void removeRenderObject(int identifier);

renderObject *getRenderObject(int identifier);

TTF_Font *loadFont(const char *pFontPath, int fontSize);

SDL_Texture *createTextTexture(const char *pText, TTF_Font *pFont, SDL_Color *pColor);

int createText(int depth, float x,float y, float width, float height, char *pText, TTF_Font *pFont, SDL_Color *pColor, bool centered);

int createImage(int depth, float x, float y, float width, float height, char *pPath, bool centered);

int createButton(int depth, float x, float y, float width, float height, char *pText, TTF_Font *pFont, SDL_Color *pColor, bool centered, char *pBackgroundPath);

// function that clears all non engine render objects (depth >= 0)
void clearAll(bool freeEngine);

void renderAll();

void initGraphics(int screenWidth,int screenHeight, int windowMode, int framecap);

void shutdownGraphics();

#endif