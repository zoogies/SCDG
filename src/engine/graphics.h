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

// function pointer for button
typedef void (*ButtonCallback)(void);

// linked list holding pointers towards button render objects
typedef struct button {
    struct renderObject *pObject;
    struct button *pNext;
    ButtonCallback callback;
} button;

void addRenderObject(int identifier, renderObjectType type, int depth, float x, float y, float width, float height, SDL_Texture *pTexture, bool centered);

void removeRenderObject(int identifier);

renderObject *getRenderObject(int identifier);

TTF_Font *loadFont(const char *pFontPath, int fontSize);

// Create a texture from image path, returns NULL for failure
SDL_Texture *createImageTexture(const char *pPath);

SDL_Texture *createTextTexture(const char *pText, TTF_Font *pFont, SDL_Color *pColor);

int createText(int depth, float x,float y, float width, float height, char *pText, TTF_Font *pFont, SDL_Color *pColor, bool centered);

int createImage(int depth, float x, float y, float width, float height, char *pPath, bool centered);

int createButton(int depth, float x, float y, float width, float height, char *pText, TTF_Font *pFont, SDL_Color *pColor, bool centered, char *pBackgroundPath, void (*callback)(void));

// function that clears all non engine render objects (depth >= 0)
void clearAll(bool freeEngine);

void removeButton(int id);

void debugForceRefresh();

void renderAll();

void checkClicked(int x, int y);

void setViewport(int screenWidth, int screenHeight);

void initGraphics(int screenWidth,int screenHeight, int windowMode, int framecap);

void shutdownGraphics();

#endif