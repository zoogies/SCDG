#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

#include "engine/engine.h"

// define screen size parameters,
// game will manage these values not engine
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// declare helper const for tracking middle coords of screen
const int SCREEN_MIDDLE_WIDTH = SCREEN_WIDTH / 2;
const int SCREEN_MIDDLE_HEIGHT = SCREEN_HEIGHT / 2;

// enum defining states as integer constants
// enum State {
//     startup = 0,
//     menu = 1
// };

// set current game state to main menu
// static int current_state = 0;

// NOTE:
// - all control loops are handled (non threaded at least) in the game file, engine is just callable interface to SDL2

/*
    PLANNING:
    - this game file will control reading the game data from whatever datastructure
      is picked, whether binary or json and act upon them

    - when state is decoupled from input in the future there needs to be an implemented lock
      to wait for like a scene to finish setting up before the user can skip so no spamming mouse
      bugging the game out
*/

int main() {
    // output game title and info
    printf("\n\033[0;33mStardust Crusaders Dating Sim v0.0.1\033[0;37m\n");

    /*
    Initialize engine, this will cover starting audio as well as splash screen
    and manage all subsequent backend rendering and audio playing invoked by the game
    */
    printf("Attempting to initialize engine... ");
    initEngine(SCREEN_WIDTH,SCREEN_HEIGHT);

    // game initialization code goes here

    // game loop code goes here

    // begin event catching
    SDL_Event e; // define new event
    bool quit = false; // define quit
    while(!quit) {
        while(SDL_PollEvent(&e)){
            if(e.type == SDL_QUIT){ // check if event is to quit
                quit = true; // quit
            }
        }
    }

    return 0; // el classico
}