/*
    TODO: GAME PLANNING:
    - track down weird vsync error
    - sensible defaults loaded into savedata.json and gamedata.json if nonexistant
*/

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <jansson.h>

#include "engine/engine.h"
#include "engine/audio.h"
#include "engine/graphics.h"
#include "game.h"

// define screen size parameters,
// game manages these values not engine, TODO: potential refactor now that engine is tracking internally
int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;

// define game volume
int VOLUME = 128;

// define window mode
// 0   -> windowed
// 1   -> fullscreen
// 16  -> borderless
// 128 -> maximized
int windowMode = 0;

// vsync is -1, all else is a number
int framecap = -1;

// flag for skipping the intro
bool skipintro = false;

// entry point to the game, which invokes all necessary engine functions by extension
int main(int argc, char *argv[]) {
    // output game title and info
    printf("\n\033[0;33mStardust Crusaders Dating Sim v0.0.1\033[0;37m\n");

    // check if we are in debug mode (from console flags)
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
            debug = true;
        } 
        if (strcmp(argv[i], "--skipintro") == 0 || strcmp(argv[i], "-s") == 0) {
            skipintro = true;
        } 
        // other flags can be implemented here in the future
    }

    /*
        Get some neccessary values from game data to startup the game
        and create them if not existing
    */

    // check if save data exists and create it if not
    // Check if the file exists
    if (access("resources/data/savedata.json", F_OK) == -1) {
        printf("Save data not found, creating...\n\n");

        // Create sensible defaults for the game data
        json_t *savedata = json_pack("{s:{s:[i,i], s:i, s:i, s:i}}",
                                      "settings",
                                      "resolution", 1920, 1080,
                                      "window mode", 1,
                                      "volume", 128,
                                      "framecap", -1);

        // Save JSON object to file
        if (savedata != NULL) {
            FILE *file = fopen("resources/data/savedata.json", "w");
            if (file != NULL) {
                json_dumpf(savedata, file, JSON_INDENT(2));
                fclose(file);
            } else {
                printf("Failed to open file: %s\n", "resources/data/savedata.json");
            }
            json_decref(savedata);
        } else {
            printf("Failed to create JSON object.\n");
        }
    } else {
        printf("Save data found, reading...\n\n");
    }

    // open the save data json
    json_error_t error;
    json_t *root = json_load_file("resources/data/savedata.json", 0, &error);
    if (!root) {
        fprintf(stderr, "Error parsing JSON file: %s\n", error.text);
        exit(1);
    }

    // Access the "settings" object
    json_t *settings = json_object_get(root, "settings");
    if (!json_is_object(settings)) {
        fprintf(stderr, "Error: 'settings' must be a JSON object\n");
        json_decref(root);
        exit(1);
    }

    // Extract volume int and validate it
    json_t *volume = json_object_get(settings, "volume");
    if (json_is_integer(volume)) {
        VOLUME = json_integer_value(volume);
        printf("Volume read from savedata: \t\t%d\n", VOLUME);
    }
    json_decref(volume);

    // Extract resolution int(s) and validate them
    json_t *resolution = json_object_get(settings, "resolution");
    if (json_is_array(resolution)) {
        // Get the first and second items from the array
        json_t *x = json_array_get(resolution, 0);
        json_t *y = json_array_get(resolution, 1);
        if (json_is_integer(x) && json_is_integer(y)) {
            SCREEN_WIDTH = json_integer_value(x);
            SCREEN_HEIGHT = json_integer_value(y);
        }
        printf("Resolution read from savedata: \t\t%dx%d\n", SCREEN_WIDTH,SCREEN_HEIGHT);
    }
    json_decref(resolution);

    // extract window mode int and validate it
    json_t *windowmode = json_object_get(settings, "window mode");
    if (json_is_integer(windowmode)) {
        windowMode = json_integer_value(windowmode);
        printf("Window Mode read from savedata: \t%d\n", windowMode);
    }
    json_decref(windowmode);

    // extract the frame cap and validate it
    json_t *savedframecap = json_object_get(settings, "framecap");
    if (json_is_integer(savedframecap)) {
        framecap = json_integer_value(savedframecap);
        printf("Frame Cap read from savedata: \t\t%d\n", framecap);
    }
    json_decref(savedframecap);

    // done with our json (for now until we eventually open it to write values)
    json_decref(root);

    /*
    Initialize engine, this will cover starting audio as well as splash screen
    and manage all subsequent backend rendering and audio playing invoked by the game
    */
    printf("\nAttempting to initialize engine... \t");
    initEngine(SCREEN_WIDTH,SCREEN_HEIGHT,debug,VOLUME,windowMode,framecap,skipintro); // this call will resolve after 2550ms of a splash screen

    /*
        ==============================
        GAME CODE ONLY BELOW THIS LINE
        ==============================
    */

    // play some main menu music on auto track looping forever
    playSound("resources/music/menu_loop.mp3",0,-1);

    // declare color and font that we are using in the game
    SDL_Color colorWhite = {255, 255, 255};
    TTF_Font *f = loadFont("resources/fonts/Nunito-Regular.ttf", 500);
    
    // add our title and mm background image to render queue 
    renderText(1,0,0,.6f,.15f,"Stardust Dating Sim",f,colorWhite,false);
    renderImage(0,.5f,.5f,1,1,"resources/images/people720.png",true);
    // TODO: add to some sort of game side queue tracking objects

    // TODO: either engine or game track specific parts of screen so its not so annyoing
    // calculating where to place things each time

    // TODO: implement button constructor to do dirty work of text orientation
    //       automatically
    // renderImage(1,550,200,250,80,"resources/images/ui/button-small.png",true);
    // renderText(2,600,210,150,60,"Play",f,colorWhite,true);

    // renderImage(1,550,300,250,80,"resources/images/ui/button-small.png");
    // renderText(2,600,210,150,60,"Scenes",f,colorWhite);

    // renderImage(1,550,400,250,80,"resources/images/ui/button-small.png");
    // renderText(2,600,210,150,60,"Settings",f,colorWhite);

    // renderImage(1,550,500,250,80,"resources/images/ui/button-small.png");
    // renderText(2,600,210,150,60,"Exit",f,colorWhite);

    // begin event catching
    SDL_Event e; // define new event
    bool quit = false; // define quit

    while(!quit) {
        while(SDL_PollEvent(&e)){ // while there is an event polled
            if(e.type == SDL_QUIT){ // check if event is to quit
                quit = true; // quit
            }
            // other input handles can be created here
            // TODO: if mouse click, run function checking if its in the bounds of
            // any active buttons (tracked by engine in a new stack, still in renderObject
            // stack however) and return the id of what has been clicked on so the game
            // (which has been tracking the objects independently) can handle it
        }
        // render frame
        renderAll();
    }

    // main game loop has finished: shutdown engine and subsequently the game
    shutdownEngine();
    printf("\033[0;31mShut down engine.\033[0;37m\n");
    printf("\033[0;31mShut down game.\033[0;37m\n");

    // graceful exit
    return 0;
}