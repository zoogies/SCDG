// includes
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

// define screen size parameters
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// enum defining states as integer constants
enum State {
    startup = 0,
    menu = 1
};

// set current game state to main menu
static int current_state = 0;

int main(int argc, char* args[]) {
    SDL_Window* window = NULL; // new window
    SDL_Surface* screenSurface = NULL; // new surface

    // subject to change MUSIC
    const char *startup = "resources/sfx/startup.mp3"; // load song
    const char *song = "resources/music/menu_loop.mp3"; // load song
    Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 640); // new mixer (i have no idea how this line works)
    Mix_Music *startupSound = Mix_LoadMUS(startup); // defining song
    Mix_Music *music = Mix_LoadMUS(song); // defining song

    // check if SDL is displaying incorrectly
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
    } 
    else {

        TTF_Init(); // init ttf

        // new window
        window = SDL_CreateWindow("Stardust Crusaders Dating Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            printf("Window creation failed: %s\n", SDL_GetError()); // catch creation error
        } 
        else {
            // TODO: FIX WINDOW ICON
            // get the window icon as a surface
            // SDL_Surface* imageSurface = IMG_Load("resources/cropped.png");

            // set window icon
            // SDL_SetWindowIcon(window, imageSurface);

            
            // int flags = MIX_INIT_MP3;
            // int result = 0;
            // if (SDL_Init(SDL_INIT_AUDIO) < 0) {
            //     printf("Failed to init SDL\n");
            //     exit(1);
            // }

            // if (flags != (result = Mix_Init(flags))) {
            //     printf("Could not initialize mixer (result: %d).\n", result);
            //     printf("Mix_Init: %s\n", Mix_GetError());
            //     exit(1);
            // }

            // get the window surface to a variable
            screenSurface = SDL_GetWindowSurface(window);

            // fill a white rectangle to the screen
            SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0, 0));
            
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

            // update window surface
            SDL_UpdateWindowSurface(window);

            // play music in mixer
            Mix_PlayMusic(startupSound, 1); // -1 value would be infinite looping
            SDL_Delay(2550); // wait for startup sound to finish
            Mix_PlayMusic(music, -1); // -1 value would be infinite looping

            SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
            SDL_UpdateWindowSurface(window);

            SDL_Event e; // define new event
            bool quit = false; // define quit
            while (!quit) {
                while (SDL_PollEvent(&e)) { // while there is a new SDL event
                    if (e.type == SDL_QUIT) { // check if its to quit
                        quit = true; // quit
                    }
                }
            }
        }
    }

    SDL_DestroyWindow(window); // destroy the window
    Mix_FreeMusic(music); // destroy the music in mixer
    SDL_Quit(); // quit SDL (should destroy anything else i forget)

    return 0;
}
