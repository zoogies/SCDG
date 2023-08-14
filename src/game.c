/*
    TODO: GAME PLANNING:
    - track down weird vsync error
*/

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <jansson.h>

#include "engine/engine.h"
#include "engine/audio.h"
#include "engine/graphics.h"
#include "engine/logging.h"
#include "engine/variant.h"

#include "game.h"
#include "discord.h"
#include "callbacks.h"
#include "data.h"
#include "state.h"
#include "scene.h"
#include "event.h"

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

// define game volume (extern)
int VOLUME = 128;

// define window mode
// SDL_FALSE                        -> windowed
// SDL_WINDOW_FULLSCREEN_DESKTOP    -> fullscreen
// SDL_WINDOW_BORDERLESS            -> borderless
// SDL_WINDOW_MAXIMIZED             -> maximized
int windowMode = SDL_FALSE;

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

bool quit = false; // define quit (extern)

TTF_Font *pStartupFont;

bool gamedebug = false;

// used to track the load time
Uint32 sceneLoadTime = 0;

// initialize state collection (extern)
StateCollection* stateCollection;

SDL_Color colorwhite = {255, 255, 255, 255};
SDL_Color colorRed = {255, 0, 0, 255};

// wrapper for engine getFont TODO remove
TTF_Font *useFont(char *key){
    return getFont(key);
}

// wrapper for engine getColor that unpacks color from json_t for us
SDL_Color *useColor(char *key, json_t *keys){
    json_t *pColorObj = getObject(getObject(keys,"color"),key);

    SDL_Color color = {getInteger(pColorObj,"r"),getInteger(pColorObj,"g"),getInteger(pColorObj,"b"),getInteger(pColorObj,"a")};

    return getColor(key, color);
}

/*
    will poll the engine for the current screen resolution and
    update the internal game globals
*/
void updateGameScreenSize(){
    struct ScreenSize size = getCurrentResolution();
    SCREEN_WIDTH = size.width;
    SCREEN_HEIGHT = size.height;

    char buffer[100];
    snprintf(buffer, sizeof(buffer), "updated game tracked screen size: %dx%d\n",SCREEN_WIDTH,SCREEN_HEIGHT);
    logMessage(debug, buffer);
}

void volumeUp(){
    // if we are max vol we dont do anything
    if(VOLUME == 128){
        return;
    }

    VOLUME += 8;
    if(VOLUME > 128){
        VOLUME = 128;
    }
    setVolume(-1,VOLUME);
    // update volume-text with "VOLUME%"
    char buffer[100];
    snprintf(buffer, sizeof(buffer),  "%d%%",(int)((float) VOLUME / 128 * 100));
    int id = getState(stateCollection, "volume-text")->intValue;
    updateText(id,buffer);
    writeInt(getObject(SAVEDATA,"settings"), "volume",VOLUME);
    saveJSONFile(SAVEDATA,"data/savedata.json");
}

void volumeDown(){
    // if we are min vol we dont do anything
    if(VOLUME == 0){
        return;
    }

    VOLUME -= 8;
    if(VOLUME < 0){
        VOLUME = 0;
    }
    setVolume(-1,VOLUME);
    char buffer[100];
    snprintf(buffer, sizeof(buffer),  "%d%%",(int)((float) VOLUME / 128 * 100));
    int id = getState(stateCollection, "volume-text")->intValue;
    updateText(id,buffer);
    writeInt(getObject(SAVEDATA,"settings"), "volume",VOLUME);
    saveJSONFile(SAVEDATA,"data/savedata.json");
}

/*
    Function to update playtime, compares the passed stamp against the global sceneLoadTime
    and adds the difference to the savedata, and resets the count
*/
void updatePlaytime(Uint32 startTime){
    // add our time played since last scene load to the playtime in savedata
    json_t *userdata = getObject(SAVEDATA,"user");
    int currentPlaytime = getInteger(userdata,"playtime");
    currentPlaytime += (int)((float)(startTime - sceneLoadTime) / 1000);
    writeInt(userdata,"playtime",currentPlaytime);
    saveJSONFile(SAVEDATA,"data/savedata.json");
    sceneLoadTime = startTime; // update our scene load time
}

// shuts down the game
int shutdownGame(){
    // update playtime
    updatePlaytime(SDL_GetTicks());

    // shutdown all scene data
    shutdownSceneManager();

    //shutdown data manager
    shutdownDataManager();

    // destroy state collection
    destroyStateCollection(stateCollection);

    // main game loop has finished: shutdown engine and subsequently the game
    shutdownEngine();

    /*
        We no longer have logging here :(
    */

    // shutdown rich presence
    shutdown_discord_rich_presence();
    
    return 0;
}

// entry point to the game, which invokes all necessary engine functions by extension
int mainFunction(int argc, char *argv[])
{
    logMessage(info, "Game started.\n");

    // create state collection
    stateCollection = createStateCollection();
    logMessage(debug, "State collection created.\n");

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

    // initialize data manager
    initializeDataManager();
    
    /*
        Get some neccessary values from game data to startup the game
        and create them if not existing
    */

    // Access the "settings" object
    json_t *settings = getObject(SAVEDATA, "settings");

    // Extract volume int and validate it
    VOLUME = getInteger(settings, "volume");

    // Extract resolution int(s) and validate them
    json_t *resolutionArray = getObject(settings, "resolution");
    SCREEN_WIDTH = getArrayInt(resolutionArray,0);
    SCREEN_HEIGHT = getArrayInt(resolutionArray,1);

    // extract window mode int and validate it
    char *mode = getString(settings,"window mode");
    if(strcmp(mode,"windowed") == 0){
        windowMode = SDL_FALSE;
    }
    else if(strcmp(mode,"fullscreen") == 0){
        windowMode = SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    else if(strcmp(mode,"borderless") == 0){
        windowMode = SDL_WINDOW_BORDERLESS;
    }
    else if(strcmp(mode,"maximized") == 0){
        windowMode = SDL_WINDOW_MAXIMIZED;
    }
    else{
        logMessage(warning, "Invalid window mode, defaulting to windowed.\n");
        windowMode = SDL_FALSE;
    }

    // extract the frame cap and validate it
    framecap = getInteger(settings, "framecap");

    /*
        Initialize engine, this will cover starting audio as well as splash screen
        and manage all subsequent backend rendering and audio playing invoked by the game
    */
    initEngine(SCREEN_WIDTH,SCREEN_HEIGHT,gamedebug,VOLUME,windowMode,framecap,skipintro); // this call will resolve after 2550ms of a splash screen

    /*
        ===============================
        START GAME CODE (no more setup)
        ===============================
    */

    // initialize color and font that we are using in the game
    pStartupFont = loadFont("fonts/Nunito-Regular.ttf", 500);

    setupSceneManager();
    loadScene("main menu");

    // for now, lets only enable rich presence on windows (im using a snap for discord which does not allow the local bridge to work)
    #ifdef _WIN32
        // initialize rich prescence
        if (init_discord_rich_presence() > 0){ // tried to make connection to discord and successfully set activity
            update_discord_activity("Playing a game", "In the main menu", "main menu", "Main Menu");
            logMessage(info, "Discord rich presence initialized.\n");
        }
        else{
            // case where user does not have a successful connection to discord
            logMessage(warning, "Discord rich presence failed to initialize.\n");
        }
    #else
        logMessage(warning, "Discord rich presence failed to initialize.\n");
    #endif

    // begin event catching
    SDL_Event e; // define new event

    /*
        Main game loop
        TODO: some things do not need run every single frame,
        like discord callbacks and maybe even handling events
    */
    while(!quit) {
        /*
            for now, lets only enable rich presence on windows (im using a snap for discord which does not allow the local bridge to work)
            Also: this probably doesnt need run every frame at all. Investigate whether this needs run just when we update or if the callbacks need
            made continuously
        */
        #ifdef _WIN32
            // something something rich presence updater
            run_discord_callbacks();
        #endif
        

        while (SDL_PollEvent(&e)) {
            handleEvent(e); // event.c
        }

        // render frame
        renderAll();
    }

    return shutdownGame(); // graceful exit
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