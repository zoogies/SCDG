#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdbool.h>

#include <SDL2/SDL_ttf.h>
#include <jansson.h>

// TODO PLEASE PLEASE MOVE TO ENGINE CALLBACK HANDLER STUFF FUNCTION FILE

struct callbackData {
    char *callbackType;
    void (*callback)(struct callbackData *data);
    json_t *pJson;
};

// linked list holding pointers towards button render objects
typedef struct button {
    bool isObject;
    struct renderObject *pObject;
    struct button *pNext;
    struct callbackData *callbackData;
} button;

// SKJDFGLKJSHFGKLFDSGKJHDFKGHDKFJHGKFDJHGKJDFHGKJHSDKHGKFSHGKFDHKGJHDKFJHG

// enum denoting all possible renderObject types
typedef enum {
    renderType_Text,
    renderType_Image,
    renderType_Button,
} renderObjectType;

typedef struct TextData {
    TTF_Font *pFont;
    int outlineSize;
    SDL_Color *pColor;
    SDL_Color *pOutlineColor;
    char *pText;
} TextData;

typedef struct ImageData {
    char *pPath;
} ImageData;

typedef struct ButtonData {
    TextData TextData; // nested text for button label
    ImageData ImageData; // nested image for button background
} ButtonData;

// struct defining renderObject(s)
typedef struct renderObject { // TODO: do we have to set each field each time or .access for only what we need
    // common to every render object
    int identifier;
    int depth;
    renderObjectType type;
    SDL_Texture *pTexture;
    SDL_Rect rect;
    struct renderObject *pNext;
    bool cachedTexture;

    // we want to remember our prior meta for reconstruction
    float relX;
    float relY;
    float relW;
    float relH;
    bool centered;

    // union holding data specific to recreating that renderObject
    union {
        TextData TextData;
        ImageData ImageData;
        ButtonData ButtonData;
    };
} renderObject;

void addRenderObject(renderObject staging);

void removeRenderObject(int identifier);

renderObject *getRenderObject(int identifier);

TTF_Font *loadFont(const char *pFontPath, int fontSize);

/*
    struct holding info on texture creations
    TODO: in the future will we need refcounts for arbitrary cache objects holding more than just their texture?
*/
struct textureInfo {
    SDL_Texture *pTexture;
    bool cached;
};

// Create a texture from image path, returns NULL for failure
struct textureInfo createImageTexture(char *pPath, bool shouldCache);

SDL_Texture *createTextTextureWithOutline();

SDL_Texture *createTextTexture(const char *pText, TTF_Font *pFont, SDL_Color *pColor);

TTF_Font *getFont(char *key);

SDL_Color *getColor(char *key, SDL_Color color);

int createText(int depth, float x,float y, float width, float height, char *pText, TTF_Font *pFont, SDL_Color *pColor, bool centered);

int createImage(int depth, float x, float y, float width, float height, char *pPath, bool centered);

int createButton(int depth, float x, float y, float width, float height, char *pText, TTF_Font *pFont, SDL_Color *pColor, bool centered, char *pBackgroundPath, struct callbackData *data);

void updateText(int id, char *pText);

void updateImage(int id, char *pSrc);

// function that clears all non engine render objects (depth >= 0)
void clearAll(bool freeEngine);

void removeButton(int id);

void debugForceRefresh();

void renderAll();

void checkClicked(int x, int y);

void setViewport(int screenWidth, int screenHeight);

void changeWindowMode(Uint32 mode);

void changeFPS(int cap);

struct ScreenSize getCurrentResolution();

void changeResolution(int width, int height);

void initGraphics(int screenWidth,int screenHeight, int windowMode, int framecap);

void shutdownGraphics();

#endif