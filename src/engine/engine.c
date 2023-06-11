/*
    TODO: ENGINE
    - engine resources (icon, fonts) need to be seperated in a sensical way from game code
      and resources. Engine should feel more seperate and able to be converted to other projects
    - (maybe) seperate some prinf output feedbacks into debug only
    - run every file in gpt4 and ask for issues
    - update all header files with the comment descriptions of the functions
*/

#include <stdio.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "audio.h"
#include "graphics.h"
#include "engine.h"

// buffer to hold filepath strings
// will be modified by getPath()
char path_buffer[1024];

// get the base path
char *base_path = NULL;

// initialize engine internal variable globals to NULL
SDL_Color *pEngineFontColor = NULL;
TTF_Font *pEngineFont = NULL;

// helper function to print a positive output
void debugOutputComplete(){
    printf("\033[0;37m"); // set color to white
    printf("(");
    printf("\033[0;32m"); // set color to green
    printf("success");
    printf("\033[0;37m"); // set color to white
    printf(")\n");
}

// helper function to get the screen size
// TODO: consider moving graphics.c
struct ScreenSize getScreenSize(){
    // initialize video
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        exit(1);
    }

    // use video to initialize display mode
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        printf("SDL_GetCurrentDisplayMode failed! SDL_Error: %s\n", SDL_GetError());
    }

    int screenWidth = displayMode.w;
    int screenHeight = displayMode.h;

    printf("Inferred sreen size: \t\t\t%dx%d\n", screenWidth, screenHeight);

    // return a ScreenSize struct with the screen width and height
    struct ScreenSize screenSize = {screenWidth, screenHeight};
    return screenSize;
}

// helper function to get the path to a resource
// will modify buffer automatically
char *getPath(char *path){
    // lazily set the base path used for opening resources cross platform on first call
    if(base_path == NULL){
        base_path = SDL_GetBasePath();
        if (base_path == NULL) {
            printf("Error getting base path: %s\n", SDL_GetError());
        }
    }

    snprintf(path_buffer, sizeof(path_buffer), "%s../../resources/%s", base_path,path);
    return path_buffer;
}

// engine entry point, takes in the screenWidth, screenHeight and a bool flag for
// starting in debug mode
void initEngine(int screenWidth, int screenHeight, bool debug, int volume, int windowMode, int framecap, bool skipintro){
    // acknowledge engine initialization
    debugOutputComplete();

    // output engine name to terminal
    printf("\n\033[0;33mYoyo Engine v0.0.1\033[0;37m\n");

    // initialize graphics systems, creating window renderer, etc
    initGraphics(screenWidth,screenHeight,windowMode,framecap);

    // load a font for use in engine (value of global in engine.h modified)
    pEngineFont = loadFont(getPath("fonts/Nunito-Bold.ttf"), 500);

    // allocate memory for and create a pointer to our engineFontColor struct for use in graphics.c
    // TODO: check this later because i'm so tired and perplexed with this workaround to letting the fn go out of scope
    SDL_Color engineFontColor = {255, 255, 0, 255};
    pEngineFontColor = &engineFontColor;
    pEngineFontColor = malloc(sizeof(SDL_Color));
    pEngineFontColor->r = 255;
    pEngineFontColor->g = 255;
    pEngineFontColor->b = 0;
    pEngineFontColor->a = 255;

    // load a SDL_Color(s) for use in engine debug displays and startup
    SDL_Color colorWhite = {255, 255, 255, 255};

    // if we are in debug mode
    if(debug){
        // display in console
        printf("\033[0;35mDebug mode enabled.\033[0;37m\n");
        
        // add fps counter manually to render stack with a custom id
        addRenderObject(-1, renderType_Text, 999, .0f, .0f, .15f, .1f, createTextTexture("fps: 0",pEngineFont,pEngineFontColor),false);
    }

    // debug output
    printf("Attempting to initialize audio... \t");

    // startup audio systems
    initAudio();

    // before we play our loud ass startup sound, lets check what volume the game wants
    // the engine to be at initially
    setVolume(-1, volume);

    /*
        Part of the engine startup which isnt configurable by the game is displaying
        a splash screen with the engine title and logo for 2550ms and playing a
        startup noise
    */
    if(skipintro){
        printf("\033[0;35mSkipping Intro.\033[0;37m\n");
    }
    else{
        playSound(getPath("sfx/startup.mp3"),0,0); // play startup sound

        // create startup logo and title and save their id# into memory to destroy them after startup
        const int engineLogo = createImage(0,.5f,.5f,.35f,.4f,getPath("images/enginelogo.png"),true);

        const int engineTitle = createText(0,.5f,.3f,.3f,.1f,"yoyo engine",pEngineFont,&colorWhite,true);

        // render everything in engine queue
        renderAll(); 

        // pause on engine splash for 2550ms (TODO: consider alternatives)
        SDL_Delay(2550); 
        
        // remove the engine logo and title from engine renderer queue
        removeRenderObject(engineLogo);
        removeRenderObject(engineTitle);
    }

    // render everything in engine queue after splash asset removal
    renderAll();

    // debug output
    printf("\n\033[0;35mEngine Fully Initialized.\033[0;37m\n\n");
} // control is now resumed by the game

// function that shuts down all engine subsystems and components ()
void shutdownEngine(){

    /*
        TECHNICALLY this might not have been called unless getPath() 
        was called at least once, however: i dont feel like fixing 
        this as the engine will always call this internally for the 
        splash screen (unless the argv for skipping is passed) in 
        which case it doesnt matter because its called in the game 
        and nobody will ever use this engine :P
    */
    SDL_free(base_path);

    // free the engine font color
    free(pEngineFontColor);
    pEngineFontColor = NULL;

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