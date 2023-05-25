#include <stdio.h>

#include <SDL2/SDL.h>

#include "audio.h"
#include "graphics.h"

// TODO calculate some global for center of the screen so that subsequent processes can use it (MAYBE PUT THIS IN GRAPHICS BUT COULD BE USEFUL HERE)

// NOTE:
// - if any object or piece of memory from a file thats not the engine, store the variable in the engine and return pointers to the memory from child files so engine file can better manage it

void debugOutputComplete(){
    printf("\033[0;37m"); // set color to white
    printf("(");
    printf("\033[0;32m"); // set color to green
    printf("initialized");
    printf("\033[0;37m"); // set color to white
    printf(")\n");
}

void initEngine(int screenWidth, int screenHeight){
    debugOutputComplete(); // debug: acknowledge engine initialization

    printf("\n\033[0;33mYoyo Engine v0.0.1\033[0;37m\n"); // output engine name to terminal

    // debug printf
    printf("Attempting to initialize window... ");

    // create new window with passed parameters
    initGraphics(screenWidth,screenHeight);

    // debug printf
    printf("Attempting to initialize audio... ");

    // startup audio systems
    initAudio(); // setup engine audio

    // part of init process is pause on engine splash for X amount of time then free back to game
    // code to show engine splash render below
    
    playSound("resources/sfx/startup.mp3",0); // play startup sound

    SDL_Delay(2550); // baked in delay to boot to game (THIS IS BAD BAD BAD BAD)
    
    // start game code
    // this call should actually be relayed down from game.c eventually

    playSound("resources/music/menu_loop.mp3",-1);
}

void shutdownEngine(){
    // shutdown graphics
    shutdownGraphics();

    // shutdown audio
    shutdownAudio();

    // quit SDL (should destroy anything else i forget)
    SDL_Quit();
}