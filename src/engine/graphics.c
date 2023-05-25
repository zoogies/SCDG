// TODO: understand this code
// also, refactor the whole thing and setup some sort of system to manage a stack
// of what needs to be rendered each frame or invokation of a render fn if we are
// leaving the rendering framerate management to the game (as it probably should be)

#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "engine.h"

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;
SDL_Renderer* renderer = NULL;

TTF_Font* loadFont(const char* fontPath, int fontSize) {
    TTF_Font* font = TTF_OpenFont(fontPath, fontSize);
    if (font == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return NULL;
    }
    return font;
}

void renderText(SDL_Renderer* renderer, const char* text, TTF_Font* font, SDL_Color color, SDL_Rect* rect) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (surface == NULL) {
        printf("Failed to render text: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return;
    }

    SDL_RenderCopy(renderer, texture, NULL, rect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void render_image(SDL_Renderer *renderer, const char* path, SDL_Rect* dest) {
    SDL_Surface *image_surface = IMG_Load(path);

    if (!image_surface)
    {
        printf("Error loading image: %s\n", IMG_GetError());
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image_surface);

    if (!texture)
    {
        printf("Error creating texture: %s\n", SDL_GetError());
    }
    else
    {
        SDL_RenderCopy(renderer, texture, NULL, dest);
    }

    SDL_FreeSurface(image_surface);
    SDL_DestroyTexture(texture);
}

void initGraphics(int screenWidth,int screenHeight){
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        exit(1);
    } 
    else {
        window = SDL_CreateWindow("Stardust Crusaders Dating Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            printf("Window creation failed: %s\n", SDL_GetError()); // catch creation error
            exit(1);
        } 
        else {
            debugOutputComplete(); // debug: acknowledge graphics initialization

            printf("Attempting to initialize renderer... ");
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (renderer == NULL) {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                exit(1);
            }

            debugOutputComplete();
            
            printf("Attempting to initialize TTF... ");

            if (TTF_Init() == -1) {
                printf("SDL2_ttf could not initialize! SDL2_ttf Error: %s\n", TTF_GetError());
                exit(1);
            }

            debugOutputComplete();
            
            // screenSurface = SDL_GetWindowSurface(window);
            // SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0, 0));
            // SDL_UpdateWindowSurface(window);
            // SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
            
            TTF_Font* font = loadFont("resources/fonts/Nunito-Bold.ttf", 500); // todo open this with the max size possible per given engine resolution at runtime TODO TODO
            if (font == NULL) {
                exit(1);
            }
            SDL_Color color = {255, 255, 255};  // White text
            SDL_Rect rect = {(1280 / 2) - 250, (1280 / 2) - (125 / 2) - 500, 500, 125};  // Text position and size
            SDL_Rect logo = {(1280 / 2) - 200, (720 / 2) - 200, 400, 400};  // Text position and size

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set background color to black (R, G, B, A)
            SDL_RenderClear(renderer); // Clear the window with the set background color
            SDL_Color background_color = {0, 0, 0};  // Black background
            renderText(renderer, "yoyo engine", font, color, &rect);

            render_image(renderer, "resources/images/enginelogo.png", &logo);

            SDL_RenderPresent(renderer);  // Update the window to show the rendered text

            SDL_UpdateWindowSurface(window);
        }
    }
}

void shutdownGraphics(){
    // shutdown all graphics and free all relevent memory
    // TTF_CloseFont(font); TODO
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
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