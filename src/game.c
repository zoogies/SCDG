/*
    TODO: GAME PLANNING:
    - track down weird vsync error
    - sensible defaults loaded into savedata.json and gamedata.json if nonexistant
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

enum scenes currentScene = mainmenu;

bool quit = false; // define quit

TTF_Font *pStartupFont;

bool gamedebug = false;

// tracks whether the console is open or not
bool consoleOpen = false;
char consoleString[100];

// used to track the load time
Uint32 sceneLoadTime = 0;

// initialize state collection
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

// function pointer functions to cover all cases:
// go to new scene
// change an internel variable by x amount or to new value
// play a sound
// increment in story defined in json?

// /*
//     will poll the engine for the current screen resolution and
//     update the internal game globals
// */
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
    printf("%s",buffer);
    json_t *SAVEDATA = getSaveData("data/savedata.json");
    writeInt(getObject(SAVEDATA,"settings"), "volume",VOLUME);
    saveJSONFile(SAVEDATA,"data/savedata.json");
    json_decref(SAVEDATA); // destroy our root json_t*
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
    printf("%s",buffer);
    json_t *SAVEDATA = getSaveData("data/savedata.json");
    writeInt(getObject(SAVEDATA,"settings"), "volume",VOLUME);
    saveJSONFile(SAVEDATA,"data/savedata.json");
    json_decref(SAVEDATA); // destroy our root json_t*
}

// TODO: take in root gamedata and not all json_t?
void constructScene(json_t *pObjects, json_t *keys, json_t *protypes){
    json_t *depthKeys = getObject(keys,"depth");
    json_t *fontKeys = getObject(keys,"font");

    // loop through all renderObjects
    // TODO: consider making its own method for protype merging
    for(size_t i = 0; i<json_array_size(pObjects); i++){
        json_t *obj = getArrayIndex(pObjects,i);
        json_t *tmp = NULL;  // Declare a temporary JSON object

        // test for prototype field and construct if existant (overwrites obj)
        char *prototypeName = getStringNOWARN(obj,"prototype");
        if(prototypeName != NULL){
            // new object for merged result
            json_t *prototype = getObject(protypes,prototypeName);

            const char *key;
            json_t *value;
            tmp = json_object();

            // iterate through prototype and add all fields that dont exist in obj
            json_object_foreach(prototype, key, value) {
                if (!json_object_get(obj, key)) {
                    json_object_set(tmp, key, value);
                }
            }
            // iterate through obj and add all fields
            json_object_foreach(obj, key, value) {
                json_object_set(tmp, key, value);
            }

            // replace obj with tmp
            obj = tmp;
        }

        char *type = getString(obj,"type");

        // TODO: will fail here before even checking type if invalid
        int depth = getInteger(depthKeys, getString(obj,"depth"));
        float x = getFloat(obj,"x");
        float y = getFloat(obj,"y");
        float w = getFloat(obj,"w");
        float h = getFloat(obj,"h");
        bool centered = getBool(obj,"centered");

        int created;
        
        if(strcmp(type,"text") == 0){
            char *text = getString(obj,"text");

            char *fontpath = getString(fontKeys,getString(obj,"font"));
            TTF_Font *pFont = getFont(fontpath);

            char *colortxt = getString(obj,"color");
            SDL_Color * pColor = useColor(colortxt,keys);
            
            created = createText(
                depth,
                x,
                y,
                w,
                h,
                text,
                pFont,
                pColor,
                centered
            );
        }
        else if(strcmp(type,"image") == 0){
            char *src = getString(obj,"src");

            created = createImage(
                depth,
                x,
                y,
                w,
                h,
                src,
                centered
            );
        }
        else if(strcmp(type,"button") == 0){
            char *src = getString(obj,"src");
            char *txt = getString(obj,"text");

            char *fonttxt = getString(getObject(keys, "font"),getString(obj,"font"));
            TTF_Font *pFont = getFont(fonttxt);

            char *colortxt = getString(obj,"color");
            SDL_Color * pColor = useColor(colortxt,keys);

            // load callback data from json
            json_t *pCallback = getObject(obj,"callback");

            // create our callback data struct and let our button know
            struct callbackData *cb = malloc(sizeof(struct callbackData));

            char *callbackType = malloc(strlen(getString(pCallback, "type")) + 1);
            strcpy(callbackType, getString(pCallback, "type"));
            cb->callbackType = callbackType;
            
            cb->callback = &callbackHandler;

            // procedurally assign callback parameters from json
            // TODO: can this be cleaned up, outsourced to callback fn

            // deep copy is important to preserve all fields
            // CREATES A NEW JSON_T WITH REFCOUNT 1
            // we HAVE TO decref this when we are done with it
            json_t *pCallbackCopy = json_deep_copy(pCallback);
            cb->pJson = pCallbackCopy;

            created = createButton(
                depth,
                x,
                y,
                w,
                h,
                txt,
                pFont,
                pColor,
                centered,
                src,
                cb
            );
        }
        else{
            logMessage(error, "Invalid renderObject type in main menu scene.\n");
            (void)created;
        }

        // check if our object has a kvp so we can be tracking it
        char *identifier = getStringNOWARN(obj,"identifier");
        if(identifier != NULL){
            // add to kvp (or update?)
            char buffer[100];
            snprintf(buffer, sizeof(buffer),  "Adding '%s' to tracked objects.\n", identifier);
            logMessage(debug, buffer);
            addState(stateCollection, identifier, (State){.type = STATE_INT, .intValue = created});
        }

        // if we created a new json with a refcount for tmp we need to decref it so it deletes
        if (tmp != NULL) {
            json_decref(tmp);
        }
    }
}

// TODO: MOVEME
enum scenes getSceneNameEnum(char *name){
    if(strcmp(name, "main menu") == 0){
        return mainmenu;
    }
    else if(strcmp(name, "settings") == 0){
        return settings;
    }
    else{
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "COULD NOT CONVERT STRING TO SCENE ENUM '%s'. RETURNING TO MAIN MENU.\n", name);
        logMessage(error, buffer);
        return mainmenu;
    }
}

/*
    Function to update playtime, compares the passed stamp against the global sceneLoadTime
    and adds the difference to the savedata, and resets the count
*/
void updatePlaytime(Uint32 startTime){
    // add our time played since last scene load to the playtime in savedata
    json_t *SAVEDATA = getSaveData("data/savedata.json");
    json_t *userdata = getObject(SAVEDATA,"user");
    int currentPlaytime = getInteger(userdata,"playtime");
    currentPlaytime += (int)((float)(startTime - sceneLoadTime) / 1000);
    writeInt(userdata,"playtime",currentPlaytime);
    saveJSONFile(SAVEDATA,"data/savedata.json");
    json_decref(SAVEDATA); // destroy our root json_t*
    sceneLoadTime = startTime; // update our scene load time
}

// TODO: FOR FASTER SCENE SWITCHING, WE POP OUT THE CURRENT RENDERLIST AND REPLACE IT WITH OUR NEW ONE TO APPEND OBJECTS TO
// OLD LIST CAN GET DESTROYED AFTER WE LOAD THE SCENE FOR LESS LATENCY (in new thread?)
void loadScene(enum scenes scene){
    // we are going to start a counter to see how long the scene takes to load
    Uint32 startTime = SDL_GetTicks(); // get the current time (we will use this also to calculate playtime)

    updatePlaytime(startTime);

    // clear all game objects to prep for switching scenes
    clearAll(false);

    // clear our state TODO: should (some) state persist?
    clearStateCollection(stateCollection);

    currentScene = scene;

    // load keys and json scenes dict
    json_t *GAMEDATA = getGameData("data/gamedata.json");
    json_t *scenes = getObject(GAMEDATA,"scenes");

    // TODO: FIXME: MAKE GLOBAL
    json_t *keys = getObject(GAMEDATA,"keys");
    json_t *channelKeys = getObject(getObject(GAMEDATA,"keys"),"channel");
    json_t *prototypes = getObject(GAMEDATA,"prototypes");

    json_t *_scene;
    json_t *pMusic;
    json_t *pObjects;


    // TODO: no switch just load from enum string?
    switch (scene)
    {
    case mainmenu:
        _scene = getObject(scenes,"main menu");
        logMessage(debug, "Loading main menu scene.\n");

        // load our music TODO: can be run in every scene
        pMusic = getObject(_scene,"music");
        playSound(getString(pMusic,"src"),getInteger(channelKeys, getString(pMusic, "channel")),getInteger(pMusic, "loops"));

        // get our scene objects and render them all
        pObjects = getArray(_scene,"renderObjects");
        constructScene(pObjects,keys,prototypes);
        break;
    case settings:
        _scene = getObject(scenes,"settings");
        logMessage(debug, "Loading settings scene.\n");

        // load our music TODO: can be run in every scene
        pMusic = getObject(_scene,"music");
        playSound(getString(pMusic,"src"),getInteger(channelKeys, getString(pMusic, "channel")),getInteger(pMusic, "loops"));

        // get our scene objects and render them all
        pObjects = getArray(_scene,"renderObjects");
        constructScene(pObjects,keys,prototypes);

        // update our volume text
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "%d%%",(int)((float) VOLUME / 128 * 100));
        int id = getState(stateCollection, "volume-text")->intValue;
        updateText(id,buffer);
        
        break;
    default:
        break;
    }
    json_decref(GAMEDATA); // free only our ROOT json, everything else is borrowed

    Uint32 endTime = SDL_GetTicks();

    // Calculate the elapsed time
    Uint32 elapsedTime = endTime - startTime;

    // Print the elapsed time in milliseconds
    char buffer[100];
    snprintf(buffer, sizeof(buffer),  "Scene loaded in %ums.\n", elapsedTime);
    logMessage(debug, buffer);
}

// shuts down the game
int shutdownGame(){
    // update playtime
    updatePlaytime(SDL_GetTicks());

    // destroy state collection
    destroyStateCollection(stateCollection);

    // main game loop has finished: shutdown engine and subsequently the game
    shutdownEngine();

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

    /*
        Get some neccessary values from game data to startup the game
        and create them if not existing
    */

    json_t *SAVEDATA = getSaveData("data/savedata.json");

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
    
    // close our ROOT json
    json_decref(SAVEDATA);

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

    // initialize color and font that we are using in the game
    pStartupFont = loadFont("fonts/Nunito-Regular.ttf", 500);

    loadScene(mainmenu);

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

    while(!quit) {
        // something something rich presence
        run_discord_callbacks();

        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: {
                    quit = true;
                    break;
                }
                case SDL_MOUSEBUTTONDOWN: {
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        int mouseX = e.button.x;
                        int mouseY = e.button.y;
                        checkClicked(mouseX, mouseY);
                        
                        char buffer[100];
                        snprintf(buffer, sizeof(buffer), "Left click event at (%d, %d)\n", mouseX, mouseY);
                        logMessage(debug, buffer);
                    }
                    break;
                }
                case SDL_KEYDOWN: {
                    switch (e.key.keysym.sym) {
                        case SDLK_BACKQUOTE: {
                            toggleOverlay();
                            break;
                        }
                        case SDLK_TAB: {
                            consoleString[0] = '>';
                            consoleString[1] = '\0';
                            toggleConsole();
                            consoleOpen = !consoleOpen;
                            break;
                        }
                        default: {
                            if (consoleOpen) {
                                if (e.key.keysym.sym != SDLK_RETURN) {
                                    if (e.key.keysym.sym == SDLK_BACKSPACE) {
                                        size_t bufferLength = strlen(consoleString);
                                        if (bufferLength > 1) {
                                            consoleString[bufferLength - 1] = '\0';
                                            updateText(-902, consoleString);
                                        }
                                    } else {
                                        if (strlen(consoleString) < 100 - 1) {
                                            strncat(consoleString, (char*)&e.key.keysym.sym, 1);
                                            updateText(-902, consoleString);
                                        } else {
                                            logMessage(error, "Buffer at max length!\n");
                                            playSound("sfx/pipe.mp3", -1, 0);
                                        }
                                    }
                                } else {
                                    char* token = strtok(consoleString, " ");
                                    if (token != NULL) {
                                        if (strcmp(token, ">load") == 0) {
                                            token = strtok(NULL, " ");
                                            if (token != NULL) {
                                                loadScene(getSceneNameEnum(token));
                                            }
                                        } else {
                                            logMessage(error, "Invalid command!\n");
                                            playSound("sfx/pipe.mp3", -1, 0);
                                        }
                                    }
                                    consoleString[0] = '>';
                                    consoleString[1] = '\0';
                                    updateText(-902, consoleString);
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
                case SDL_WINDOWEVENT: {
                    if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                        // Reset the viewport when the game window regains focus
                        // setViewport(SCREEN_WIDTH, SCREEN_HEIGHT);
                    }
                    break;
                }
            }
        }
        // render frame
        renderAll();
    }

    // we dont get logging here as its already been shutdown but should be fine
    // TODO: persist logging
    // give it its own sdl instance path or some failsafe to write to a file
    
    // graceful exit
    return shutdownGame();
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