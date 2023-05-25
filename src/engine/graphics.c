#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "engine.h"

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;

void initGraphics(int screenWidth,int screenHeight){
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        exit(1);
    } 
    else {
        window = SDL_CreateWindow("Stardust Crusaders Dating Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            printf("Window creation failed: %s\n", SDL_GetError()); // catch creation error
        } 
        else {
            debugOutputComplete(); // debug: acknowledge graphics initialization
            screenSurface = SDL_GetWindowSurface(window);
            SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0, 0));
            // SDL_UpdateWindowSurface(window);
            // SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
            SDL_UpdateWindowSurface(window);
        }
    }
}

void shutdownGraphics(){
    // shutdown all graphics and free all relevent memory
    SDL_DestroyWindow(window);
}
    
// TODO: font stuff
// const char *FONT_PATH = "resources/fonts/Nunito-Regular.ttf";
// const int FONT_SIZE = 24;
// SDL_Color textColor = {0, 0, 0, 255};

// TODO: font stuff
// TTF_Font* font = TTF_OpenFont(FONT_PATH,FONT_SIZE);
// SDL_Surface* surface = TTF_RenderText_Solid(font, "Hello, World!", textColor);
// SDL_Texture* texture = SDL_CreateTextureFromSurface(window, surface);
// SDL_Rect destinationRect = {0, 0, surface->w, surface->h};
// SDL_RenderCopy(window, texture, NULL, &destinationRect);