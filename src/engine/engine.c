#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "audio.h"
#include "graphics.h"

// NOTE:
// - if any object or piece of memory from a file thats not the engine, store the variable in the engine and return pointers to the memory from child files so engine file can better manage it

// declare helper const for tracking middle coords of screen
int SCREEN_MIDDLE_WIDTH = 0;
int SCREEN_MIDDLE_HEIGHT = 0;

// font used by engine
TTF_Font *NunitoBold = NULL;

void debugOutputComplete(){
    printf("\033[0;37m"); // set color to white
    printf("(");
    printf("\033[0;32m"); // set color to green
    printf("success");
    printf("\033[0;37m"); // set color to white
    printf(")\n");
}

void initEngine(int screenWidth, int screenHeight, bool debug){
    // im so tired
    SCREEN_MIDDLE_WIDTH = screenWidth / 2;
    SCREEN_MIDDLE_HEIGHT = screenHeight / 2;

    debugOutputComplete(); // debug: acknowledge engine initialization

    printf("\n\033[0;33mYoyo Engine v0.0.1\033[0;37m\n"); // output engine name to terminal

    // create new window with passed parameters
    initGraphics(screenWidth,screenHeight);

    NunitoBold = loadFont("resources/fonts/Nunito-Bold.ttf", 500);

    SDL_Color colorYellow = {255, 255, 0};
    if(debug){
        printf("\033[0;35mDebug mode enabled.\033[0;37m\n");
        
        addRenderObject(99,99,0,-10,100,50,createTextTexture("fps",NunitoBold,colorYellow));
    }

    // debug printf
    printf("Attempting to initialize audio... \t");

    // startup audio systems
    initAudio(); // setup engine audio

    // part of init process is pause on engine splash for X amount of time then free back to game
    // code to show engine splash render below
    
    playSound("resources/sfx/startup.mp3",0); // play startup sound

    SDL_Color colorWhite = {255, 255, 255};  // White // TODO move mee im too tired

    // add two engine specific renderObjects here
    addRenderObject(0,0,(SCREEN_MIDDLE_WIDTH - 200),(SCREEN_MIDDLE_HEIGHT - 200),400,400,createImageTexture("resources/images/enginelogo.png"));
    addRenderObject(1,0,(SCREEN_MIDDLE_WIDTH - 200),(SCREEN_MIDDLE_HEIGHT - 300),400,150,createTextTexture("yoyo engine", NunitoBold, colorWhite)); // TODO fix font

    renderAll(); // render everything in linked list storage

    SDL_Delay(2550); // baked in delay to boot to game (THIS IS BAD BAD BAD BAD)
    
    removeRenderObject(0); // remove engine logo
    removeRenderObject(1); // remove engine title

    renderAll();

    // start game code
    printf("\n\033[0;35mEngine Fully Initialized.\033[0;37m\n\n");
}

void shutdownEngine(){
    // shutdown graphics
    shutdownGraphics();

    // shutdown audio
    shutdownAudio();

    // quit SDL (should destroy anything else i forget)
    SDL_Quit();
}