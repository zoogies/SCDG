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
#include "engine/logging.h"

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
    logMessage(info, "Game started.\n");

    // check if we are in debug mode (from console flags)
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-d") == 0) {
            gamedebug = true;
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
        logMessage(warning, "Save data not found, creating...\n");

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
                logMessage(error, "Failed to open save data file.\n");
            }
            json_decref(pSaveData);
        }
        else {
            logMessage(error,"failed to create JSON object.\n");
        }
    }
    else {
        logMessage(info, "Save data found, reading...\n");
    }

    // open the save data json
    json_error_t err;
    json_t *pRoot = json_load_file(getPath("data/savedata.json"), 0, &err);
    if (!pRoot) {
        logMessage(error, "Error parsing JSON file.\n");
        exit(1);
    }

    // Access the "settings" object
    json_t *pSettings = json_object_get(pRoot, "settings");
    if (!json_is_object(pSettings)) {
        logMessage(error, "Error settings is not json object.\n");
        json_decref(pRoot);
        exit(1);
    }

    // Extract volume int and validate it
    json_t *pVolume = json_object_get(pSettings, "volume");
    if (json_is_integer(pVolume)) {
        VOLUME = json_integer_value(pVolume);
        char buffer[100];
        sprintf(buffer, "Volume read from savedata: %d/128\n", VOLUME);
        logMessage(debug, buffer);
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
        char buffer[100];
        sprintf(buffer, "Resolution read from savedata: %dx%d\n", SCREEN_WIDTH,SCREEN_HEIGHT);
        logMessage(debug, buffer);
    }
    json_decref(pResolution);

    // extract window mode int and validate it
    json_t *pWindowMode = json_object_get(pSettings, "window mode");
    if (json_is_integer(pWindowMode)) {
        windowMode = json_integer_value(pWindowMode);
        char buffer[100];
        sprintf(buffer, "Window Mode read from savedata: %d\n", windowMode);
        logMessage(debug, buffer);
    }
    json_decref(pWindowMode);

    // extract the frame cap and validate it
    json_t *pSavedFrameCap = json_object_get(pSettings, "framecap");
    if (json_is_integer(pSavedFrameCap)) {
        framecap = json_integer_value(pSavedFrameCap);
        char buffer[100];
        sprintf(buffer, "Frame Cap read from savedata: %d\n", framecap);
        logMessage(debug, buffer);
    }
    json_decref(pSavedFrameCap);

    // done with our json (for now until we eventually open it to write values)
    json_decref(pRoot);

    /*
    Initialize engine, this will cover starting audio as well as splash screen
    and manage all subsequent backend rendering and audio playing invoked by the game
    */
    initEngine(SCREEN_WIDTH,SCREEN_HEIGHT,gamedebug,VOLUME,windowMode,framecap,skipintro); // this call will resolve after 2550ms of a splash screen

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
    if (init_discord_rich_presence() > 0){ // tried to make connection to discord and successfully set activity
        update_discord_activity("Playing a game", "In the main menu", "mainmenu", "Main Menu");
        logMessage(info, "Discord rich presence initialized.\n");
    }
    else{
        // case where user does not have a successful connection to discord
        logMessage(warning, "Discord rich presence failed to initialize.\n");
    }
    // begin event catching
    SDL_Event e; // define new event
    bool quit = false; // define quit

    while(!quit) {
        // something something rich presence
        run_discord_callbacks();

        while(SDL_PollEvent(&e)){ // while there is an event polled
            //TODO: optimization would be to make this chain of else ifs into a switch
            if(e.type == SDL_QUIT){ // check if event is to quit
                quit = true; // quit
            }
            // for now, we will only check bounds if there was a click,
            // this is subject to change when highlight interactions are introduced
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    int mouseX = e.button.x;
                    int mouseY = e.button.y;
                    // run checks on if button was clicked and get its id if we did
                    int buttonClicked = checkClicked(mouseX, mouseY);
                    if(buttonClicked != intFail){
                        char buffer[100];
                        sprintf(buffer, "Left click event at (%d, %d) hit button id#%d\n", mouseX, mouseY,buttonClicked);
                        logMessage(debug, buffer);
                    }
                    else{
                        char buffer[100];
                        sprintf(buffer, "Left click event at (%d, %d) miss\n", mouseX, mouseY);
                        logMessage(debug, buffer);
                    }

                    // handle button clicks
                    if(buttonClicked == 6){
                        quit = true;
                    }
                }
            }
            else if (e.type == SDL_KEYDOWN) {
                // if key is backtick we want to toggle debug overlay
                if (e.key.keysym.sym == SDLK_BACKQUOTE) {
                    // Backtick key is pressed
                    toggleOverlay();
                }
            }
            else if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    // Reset the viewport when the game window regains focus
                    setViewport(SCREEN_WIDTH, SCREEN_HEIGHT);
                }
            }
        }
        // render frame
        renderAll();
    }

    // shut down our own game specific stuff
    TTF_CloseFont(pStartupFont);

    // main game loop has finished: shutdown engine and subsequently the game
    shutdownEngine();

    // shutdown rich presence
    shutdown_discord_rich_presence();

    // we dont get logging here as its already been shutdown but should be fine
    // TODO: persist logging
    // give it its own sdl instance path or some failsafe to write to a file
    
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