#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "engine/engine.h"
#include "engine/audio.h"
#include "engine/graphics.h"

// define screen size parameters,
// game will manage these values not engine
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// allow passed in through cli TODO
bool debug = false;

// declare helper const for tracking middle coords of screen
// const int SCREEN_MIDDLE_WIDTH = SCREEN_WIDTH / 2;
// const int SCREEN_MIDDLE_HEIGHT = SCREEN_HEIGHT / 2;

/*
    PLANNING:
    - need settings file to read from for: volume, resolution, frame sync
*/

int main(int argc, char *argv[]) {
    // output game title and info
    printf("\n\033[0;33mStardust Crusaders Dating Sim v0.0.1\033[0;37m\n");

    // check if we are in debug mode
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
            debug = true;
            break;
        }
    }

    /*
    Initialize engine, this will cover starting audio as well as splash screen
    and manage all subsequent backend rendering and audio playing invoked by the game
    */
    printf("Attempting to initialize engine... \t");
    initEngine(SCREEN_WIDTH,SCREEN_HEIGHT,debug);

    // game initialization code goes here

    // game loop code goes here
    playSound("resources/music/menu_loop.mp3",-1); // eventually needs channel parameter

    // TODO reformat graphics code to take in a passed struct instead of vice versa
    SDL_Color colorWhite = {255, 255, 255};
    addRenderObject(0,0,0,0,1280,720,createImageTexture("resources/images/people720.png"));
    addRenderObject(1,1,0,0,800,150,createTextTexture("Stardust Dating Sim",loadFont("resources/fonts/Nunito-Regular.ttf", 500),colorWhite));

    renderAll();


    // begin event catching
    SDL_Event e; // define new event
    bool quit = false; // define quit
    while(!quit) {
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT){ // check if event is to quit
                quit = true; // quit
            }
        }
        renderAll(); // render frame
    } // TODO, once you are ever reading this and working on the actual game
    // (i hope): go double check all header files

    return 0; // el classico
}