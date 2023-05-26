// TODO: understand this code
// also, refactor the whole thing and setup some sort of system to manage a stack
// of what needs to be rendered each frame or invokation of a render fn if we are
// leaving the rendering framerate management to the game (as it probably should be)

#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "engine.h"

// NOTE: below lines have been commented out as of right now,
// there is no distinction when rendering between text and image

// enum holding the types of render object
// typedef enum {
//      renderType_Text,
//      renderType_Image,
// } renderObjectType;

// struct renderObject, used for defining properties of a render object
// to store in a linked list
typedef struct renderObject {
    int identifier;
    // renderObjectType type;
    SDL_Texture* texture;
    SDL_Rect rect;
    int depth;
    struct renderObject* next;
} renderObject;

// define some globals to this file
SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;
SDL_Renderer* renderer = NULL;
renderObject* renderListHead = NULL;

// TODO: consider having images in engine start at their center, so game has to do no calculations for finding middles of things
void addRenderObject(int identifier, int depth, int x, int y, int width, int height, SDL_Texture *texture) {
    printf("Attempting to add render object [id %d]\t",identifier);
    SDL_Rect rect = {x, y, width, height};

    renderObject *obj = (renderObject *)malloc(sizeof(renderObject));
    obj->identifier = identifier;
    obj->texture = texture;
    obj->rect = rect;
    obj->depth = depth;
    obj->next = NULL;

    if (renderListHead == NULL || obj->depth < renderListHead->depth) {
        obj->next = renderListHead;
        renderListHead = obj;
    } else {
        renderObject* current = renderListHead;
        while (current->next != NULL && current->next->depth < obj->depth) {
            current = current->next;
        }
        obj->next = current->next;
        current->next = obj;
    }
    debugOutputComplete();
}

void removeRenderObject(int identifier) {
    if (renderListHead == NULL) {
        return;
    }

    if (renderListHead->identifier == identifier) {
        renderObject* toDelete = renderListHead;
        renderListHead = renderListHead->next;
        SDL_DestroyTexture(toDelete->texture);
        free(toDelete);
        return;
    }

    renderObject* current = renderListHead;
    while (current->next != NULL && current->next->identifier != identifier) {
        current = current->next;
    }
    if (current->next != NULL) {
        renderObject* toDelete = current->next;
        current->next = toDelete->next;
        SDL_DestroyTexture(toDelete->texture);
        free(toDelete);
    }
}

void updateObjectRect(int identifier, SDL_Rect* newRect) {
    renderObject* current = renderListHead;
    while (current != NULL) {
        if (current->identifier == identifier) {
            current->rect = *newRect;
            break;
        }
        current = current->next;
    }
}

void renderAll() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color to black (R, G, B, A)
    SDL_RenderClear(renderer); // Clear the window with the set background color

    renderObject* current = renderListHead;
    while (current != NULL) {
        SDL_RenderCopy(renderer, current->texture, NULL, &(current->rect));
        current = current->next;
    }
    SDL_RenderPresent(renderer);  // Update the window to show the rendered text

    SDL_UpdateWindowSurface(window);
}

// load a font into memory and return a pointer to it
TTF_Font* loadFont(const char* fontPath, int fontSize) {
    // TODO: attempting to open resource printf with opened status yellow in term
    TTF_Font* font = TTF_OpenFont(fontPath, fontSize);
    if (font == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return NULL;
    }
    return font;
}


// Create a texture from text string with specified font and color
SDL_Texture* createTextTexture(const char* text, TTF_Font* font, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (surface == NULL) {
        printf("Failed to render text: %s\n", TTF_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_FreeSurface(surface);
    return texture;
}


// Create a texture from image path
SDL_Texture* createImageTexture(const char* path) {
    SDL_Surface* image_surface = IMG_Load(path);
    if (!image_surface) {
        printf("Error loading image: %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image_surface);
    if (!texture) {
        printf("Error creating texture: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_FreeSurface(image_surface);
    return texture;
}

// initialize graphics
void initGraphics(int screenWidth,int screenHeight){
    printf("Attempting to initialize SDL... \t");
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        exit(1);
    }
    debugOutputComplete(); // debug: acknowledge graphics initialization

    printf("Attempting to initialize window... \t");
    window = SDL_CreateWindow("Stardust Crusaders Dating Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window creation failed: %s\n", SDL_GetError()); // catch creation error
        exit(1);
    }
    debugOutputComplete(); // debug: acknowledge graphics initialization


    printf("Attempting to initialize renderer... \t");
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        exit(1);
    }
    debugOutputComplete();
    

    printf("Attempting to initialize TTF... \t");
    if (TTF_Init() == -1) {
        printf("SDL2_ttf could not initialize! SDL2_ttf Error: %s\n", TTF_GetError());
        exit(1);
    }
    debugOutputComplete();
}

void shutdownGraphics(){
    // shutdown all graphics and free all relevent memory
    // TTF_CloseFont(font); TODO
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}