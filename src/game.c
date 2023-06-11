/*
    TODO: GAME PLANNING:
    - track down weird vsync error
    - sensible defaults loaded into savedata.json and gamedata.json if nonexistant
*/

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <jansson.h>

#include "engine/engine.h"
#include "engine/audio.h"
#include "engine/graphics.h"

#include "game.h"
#include "discord.h"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <string.h>
#endif

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

// enum for general depth values
enum depth {
    engineUI = 999,
    UI = 99,
    background = 0
};

// entry point to the game, which invokes all necessary engine functions by extension
int mainFunction(int argc, char *argv[])
{
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
    if (access(getPath("data/savedata.json"), F_OK) == -1) {
        printf("\n%s\n",getPath("data/savedata.json"));
        printf("Save data not found, creating...\n\n");

        // we need to get the screen size to set the defualt resolution
        struct ScreenSize size = getScreenSize();

        // Create sensible defaults for the game data
        json_t *pSaveData = json_pack("{s:{s:[i,i], s:i, s:i, s:i}}",
                                      "settings",
                                      "resolution", size.width, size.height,
                                      "window mode", 1,
                                      "volume", 128,
                                      "framecap", -1);

        // Save JSON object to file
        if (pSaveData != NULL) {
            FILE *pFile = fopen(getPath("data/savedata.json"), "w");
            if (pFile != NULL) {
                json_dumpf(pSaveData, pFile, JSON_INDENT(2));
                fclose(pFile);
            }
            else {
                printf("Failed to open file: %s\n", "resources/data/savedata.json");
            }
            json_decref(pSaveData);
        }
        else {
            printf("Failed to create JSON object.\n");
        }
    }
    else {
        printf("Save data found, reading...\n\n");
    }

    // open the save data json
    json_error_t error;
    json_t *pRoot = json_load_file(getPath("data/savedata.json"), 0, &error);
    if (!pRoot) {
        fprintf(stderr, "Error parsing JSON file: %s\n", error.text);
        exit(1);
    }

    // Access the "settings" object
    json_t *pSettings = json_object_get(pRoot, "settings");
    if (!json_is_object(pSettings)) {
        fprintf(stderr, "Error: 'settings' must be a JSON object\n");
        json_decref(pRoot);
        exit(1);
    }

    // Extract volume int and validate it
    json_t *pVolume = json_object_get(pSettings, "volume");
    if (json_is_integer(pVolume)) {
        VOLUME = json_integer_value(pVolume);
        printf("Volume read from savedata: \t\t%d\n", VOLUME);
    }
    json_decref(pVolume);

    // Extract resolution int(s) and validate them
    json_t *pResolution = json_object_get(pSettings, "resolution");
    if (json_is_array(pResolution)) {
        // Get the first and second items from the array
        json_t *pX = json_array_get(pResolution, 0);
        json_t *pY = json_array_get(pResolution, 1);
        if (json_is_integer(pX) && json_is_integer(pY)) {
            SCREEN_WIDTH = json_integer_value(pX);
            SCREEN_HEIGHT = json_integer_value(pY);
        }
        printf("Resolution read from savedata: \t\t%dx%d\n", SCREEN_WIDTH,SCREEN_HEIGHT);
    }
    json_decref(pResolution);

    // extract window mode int and validate it
    json_t *pWindowMode = json_object_get(pSettings, "window mode");
    if (json_is_integer(pWindowMode)) {
        windowMode = json_integer_value(pWindowMode);
        printf("Window Mode read from savedata: \t%d\n", windowMode);
    }
    json_decref(pWindowMode);

    // extract the frame cap and validate it
    json_t *pSavedFrameCap = json_object_get(pSettings, "framecap");
    if (json_is_integer(pSavedFrameCap)) {
        framecap = json_integer_value(pSavedFrameCap);
        printf("Frame Cap read from savedata: \t\t%d\n", framecap);
    }
    json_decref(pSavedFrameCap);

    // done with our json (for now until we eventually open it to write values)
    json_decref(pRoot);

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
    playSound(getPath("music/menu_loop.mp3"),0,-1);

    // declare color and font that we are using in the game
    SDL_Color colorWhite = {255, 255, 255, 255};
    TTF_Font *pStartupFont = loadFont(getPath("fonts/Nunito-Regular.ttf"), 500);
    
    // add our title and mm background image to render queue 
    createText(1,0,0,.6f,.15f,"Stardust Dating Sim",pStartupFont,&colorWhite,false);
    createImage(background,.5f,.5f,1,1,getPath("images/people720.png"),true);
    // add our mm buttons
    createButton(UI,.4,.25,.2,.1,"Play",pStartupFont,&colorWhite,false,getPath("images/ui/button-small.png"));
    createButton(UI,.4,.4,.2,.1,"Settings",pStartupFont,&colorWhite,false,getPath("images/ui/button-small.png"));
    createButton(UI,.4,.55,.2,.1,"Quit",pStartupFont,&colorWhite,false,getPath("images/ui/button-small.png"));

    // TODO: either engine or game track specific parts of screen so its not so annyoing
    // calculating where to place things each time
    // ex: (character pos 1-3: denote different rects to paint characters in and can be abstracted)

    // initialize rich prescence
    printf("\nAttempting to set activity status... \t");
    if (init_discord_rich_presence() > 0){ // tried to make connection to discord and successfully set activity
        update_discord_activity("Playing a game", "In the main menu", "mainmenu", "Main Menu");
        debugOutputComplete();
    }
    else{
        // case where user does not have a successful connection to discord
        printf("Failed!\n");
    }
    // begin event catching
    SDL_Event e; // define new event
    bool quit = false; // define quit

    while(!quit) {
        // something something rich presence
        run_discord_callbacks();

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

    // shutdown rich presence
    shutdown_discord_rich_presence();
    printf("\033[0;31mShut down rich presence.\033[0;37m\n");

    // graceful exit
    return 0;
}

// make sure when compiling regardless of platform main has correct entry point
#ifdef _WIN32
    int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
    {
        return mainFunction(__argc, __argv);
    }
#else
    int main(int argc, char *argv[])
    {
        return mainFunction(argc, argv);
    }
#endif