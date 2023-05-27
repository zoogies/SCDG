/*
    ENGINE TODO:
    - engine resources (icon, fonts) need to be seperated in a sensical way from game code
      and resources. Engine should feel more seperate and able to be converted to other projects
    - (maybe) seperate some prinf output feedbacks into debug only
*/

#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "audio.h"
#include "graphics.h"
#include "engine.h"

// initialize the extern variables from engine.h to locate the midpoint of the screen
int SCREEN_MIDDLE_WIDTH = 0;
int SCREEN_MIDDLE_HEIGHT = 0;

// declare font used by engine
TTF_Font *NunitoBold = NULL;

// helper function to print a positive output
void debugOutputComplete(){
    printf("\033[0;37m"); // set color to white
    printf("(");
    printf("\033[0;32m"); // set color to green
    printf("success");
    printf("\033[0;37m"); // set color to white
    printf(")\n");
}

// engine entry point, takes in the screenWidth, screenHeight and a bool flag for
// starting in debug mode
void initEngine(int screenWidth, int screenHeight, bool debug){

    // initialzie our helper variables for the midpoints on both axis of
    // the screen
    SCREEN_MIDDLE_WIDTH = screenWidth / 2;
    SCREEN_MIDDLE_HEIGHT = screenHeight / 2;

    // acknowledge engine initialization
    debugOutputComplete();

    // output engine name to terminal
    printf("\n\033[0;33mYoyo Engine v0.0.1\033[0;37m\n");

    // initialize graphics systems, creating window renderer, etc
    initGraphics(screenWidth,screenHeight);

    // load a font for use in engine
    NunitoBold = loadFont("resources/fonts/Nunito-Bold.ttf", 500);
    
    // load a SDL_Color(s) for use in engine debug displays and startup
    SDL_Color colorYellow = {255, 255, 0};
    SDL_Color colorWhite = {255, 255, 255};

    // if we are in debug mode
    if(debug){
        // display in console
        printf("\033[0;35mDebug mode enabled.\033[0;37m\n");
        
        // add
        renderText(999,0,-10,125,50,"fps: ",NunitoBold, colorYellow);
    }

    // debug output
    printf("Attempting to initialize audio... \t");

    // startup audio systems
    initAudio();

    /*
        Part of the engine startup which isnt configurable by the game is displaying
        a splash screen with the engine title and logo for 2550ms and playing a
        startup noise
    */
    playSound("resources/sfx/startup.mp3",0); // play startup sound

    // create startup logo and title and save their id# into memory to destroy them after startup
    const int engineLogo = renderImage(0,(SCREEN_MIDDLE_WIDTH - 200),(SCREEN_MIDDLE_HEIGHT - 200),400,400,"resources/images/enginelogo.png");
    const int engineTitle = renderText(0,(SCREEN_MIDDLE_WIDTH - 250),(SCREEN_MIDDLE_HEIGHT - 300),500,150,"yoyo engine",NunitoBold,colorWhite);

    // render everything in engine queue
    renderAll(); 

    // pause on engine splash for 2550ms (TODO: consider alternatives)
    SDL_Delay(2550); 
    
    // remove the engine logo and title from engine renderer queue
    removeRenderObject(engineLogo);
    removeRenderObject(engineTitle);

    // render everything in engine queue after splash asset removal
    renderAll();

    // debug output
    printf("\n\033[0;35mEngine Fully Initialized.\033[0;37m\n\n");
} // control is now resumed by the game

// function that shuts down all engine subsystems and components ()
void shutdownEngine(){
    // shutdown graphics
    shutdownGraphics();
    printf("\033[0;31mShut down graphics.\033[0;37m\n");

    // shutdown audio
    shutdownAudio();
    printf("\033[0;31mShut down audio.\033[0;37m\n");

    // quit SDL (should destroy anything else i forget)
    SDL_Quit();
    printf("\033[0;31mShut down SDL.\033[0;37m\n");
}