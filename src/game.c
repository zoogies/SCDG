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
#include "callbacks.h"
#include "data.h"

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

enum scenes currentScene = mainmenu;

bool quit = false; // define quit

SDL_Color colorWhite;
TTF_Font *pStartupFont;

// variables used a lot (must be freed in closedown)
char *smallButton;

bool gamedebug = false;

// function pointer functions to cover all cases:
// go to new scene
// change an internel variable by x amount or to new value
// play a sound
// increment in story defined in json?

void callback(){
    // generic callback does nothing TODO
}

// todo should prob go to other file
void updateText(char *key, char *text){
    SDL_Color colorWhite = {255, 255, 255, 255}; // FIXME
    int id = getValue(key);
    renderObject *pObject = getRenderObject(id);
    SDL_Texture *temp = pObject->pTexture;
    pObject->pTexture = createTextTexture(text,pStartupFont,&colorWhite);
    SDL_DestroyTexture(temp);
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
    sprintf(buffer, "%d%%",(int)((float) VOLUME / 128 * 100));
    updateText("volume-text",buffer);

    // write changes to save data
    writeInt(getObject(initSaveData(getPathStatic("data/savedata.json")), "settings"),"volume",VOLUME);
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
    sprintf(buffer, "%d%%",(int)((float) VOLUME / 128 * 100));
    updateText("volume-text",buffer);

    // write changes to save data
    writeInt(getObject(initSaveData(getPathStatic("data/savedata.json")), "settings"),"volume",VOLUME);
}

// TODO: FOR FASTER SCENE SWITCHING, WE POP OUT THE CURRENT RENDERLIST AND REPLACE IT WITH OUR NEW ONE TO APPEND OBJECTS TO
// OLD LIST CAN GET DESTROYED AFTER WE LOAD THE SCENE FOR LESS LATENCY
void loadScene(enum scenes scene){
    // clear all game objects to prep for switching scenes
    clearAll(false);
    freeTrackedObjects();

    currentScene = scene;
    
    SDL_Color colorWhite = {255, 255, 255, 255}; // FIXME
    switch (scene)
    {
    case mainmenu:
        logMessage(debug, "Loading main menu scene.\n");
        // play some main menu music on auto track looping forever
        playSound(getPathStatic("music/menu_loop.mp3"),0,-1);

        // add our title and mm background image to render queue 
        createText(1,0,0,.6f,.15f,"Stardust Dating Sim",pStartupFont,&colorWhite,false);
        createImage(background,.5f,.5f,1,1,getPathStatic("images/backgrounds/people720.png"),true);
        
        // add our mm buttons
        int playbtn = createButton(UI,.4,.25,.2,.1,"Play",pStartupFont,&colorWhite,false,smallButton,&callback);
        int settingsbtn = createButton(UI,.4,.4,.2,.1,"Settings",pStartupFont,&colorWhite,false,smallButton,&gotoSettings);
        int quitbtn = createButton(UI,.4,.55,.2,.1,"Quit",pStartupFont,&colorWhite,false,smallButton,&quitGame);
        // addObject("playbtn",playbtn);
        // addObject("settingsbtn",settingsbtn);
        // addObject("quitbtn",quitbtn);
        break;
    case settings:
        logMessage(debug, "Loading settings scene.\n");
        playSound(getPathStatic("music/settings.mp3"),0,-1);
        createText(1,0,0,.45f,.15f,"Settings",pStartupFont,&colorWhite,false);
        createImage(background,.5f,.5f,1,1,getPathStatic("images/backgrounds/settingsbg.jpg"),true);
        createButton(UI,.4,.8,.2,.1,"Exit",pStartupFont,&colorWhite,false,smallButton,&gotoMainMenu);
        
        // resolution settings
        float resY = .15f;
        createText(1,0,resY-.05,.2f,.15f,"Resolution:",pStartupFont,&colorWhite,false);
        createButton(UI,.21,resY,.15,.08,"1280x720",pStartupFont,&colorWhite,false,smallButton,&callback);
        createButton(UI,.41,resY,.15,.08,"1920x1080",pStartupFont,&colorWhite,false,smallButton,&callback);
        createButton(UI,.61,resY,.15,.08,"2560x1440",pStartupFont,&colorWhite,false,smallButton,&callback);
        createButton(UI,.81,resY,.15,.08,"3440x1440",pStartupFont,&colorWhite,false,smallButton,&callback);

        // window settings
        float winY = .25f;
        createText(1,0,winY,.2f,.1f,"Window:",pStartupFont,&colorWhite,false);
        createButton(UI,.21,winY,.15,.08,"fullscreen",pStartupFont,&colorWhite,false,smallButton,&callback);
        createButton(UI,.41,winY,.15,.08,"borderless",pStartupFont,&colorWhite,false,smallButton,&callback);
        createButton(UI,.61,winY,.15,.08,"maximized",pStartupFont,&colorWhite,false,smallButton,&callback);
        createButton(UI,.81,winY,.15,.08,"default",pStartupFont,&colorWhite,false,smallButton,&callback);

        // fps settings
        float fpsY = .35f;
        createText(1,0,fpsY,.2f,.1f,"FPS:",pStartupFont,&colorWhite,false);
        createButton(UI,.21,fpsY,.15,.08,"60",pStartupFont,&colorWhite,false,smallButton,&callback);
        createButton(UI,.41,fpsY,.15,.08,"144",pStartupFont,&colorWhite,false,smallButton,&callback);
        createButton(UI,.61,fpsY,.15,.08,"vsync",pStartupFont,&colorWhite,false,smallButton,&callback);
        createButton(UI,.81,fpsY,.15,.08,"uncapped",pStartupFont,&colorWhite,false,smallButton,&callback);

        // volume settings
        float volY = .45f;
        createText(1,0,volY,.2f,.1f,"Volume:",pStartupFont,&colorWhite,false);
        createButton(UI,.21,volY,.15,.08,"-",pStartupFont,&colorWhite,false,smallButton,&volumeDown);

        char buffer[100];
        sprintf(buffer, "%d%%",(int)((float) VOLUME / 128 * 100));
        int vtxt = createText(1,.41,volY,.15f,.08f,buffer,pStartupFont,&colorWhite,false);
        addObject("volume-text",vtxt);

        createButton(UI,.61,volY,.15,.08,"+",pStartupFont,&colorWhite,false,smallButton,&volumeUp);
        logMessage(debug, "Finished loading settings scene.\n");
        break;
    default:
        break;
    }
}

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

    json_t *pRoot = initSaveData(getPathStatic("data/savedata.json"));
    // printf("\n%s\n",json_dumps(pRoot, JSON_INDENT(2)));
    // json_decref(pRoot);
    // exit(0);

    // Access the "settings" object
    json_t *pSettings = getObject(pRoot, "settings");

    // Extract volume int and validate it
    VOLUME = getInteger(pSettings, "volume");

    // Extract resolution int(s) and validate them
    json_t *pResArray = getObject(pSettings, "resolution");
    SCREEN_WIDTH = getArrayInt(pResArray,0);
    SCREEN_HEIGHT = getArrayInt(pResArray,1);

    // extract window mode int and validate it
    windowMode = getInteger(pSettings, "window mode");

    // extract the frame cap and validate it
    framecap = getInteger(pSettings, "framecap");

    // done with our json (for now until we eventually open it to write values)
    json_decref(pSettings);
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

    // initialize color and font that we are using in the game
    pStartupFont = loadFont(getPathStatic("fonts/Nunito-Regular.ttf"), 500);

    smallButton = getPathDynamic("images/ui/button-small.png");

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
                    checkClicked(mouseX, mouseY);
                    //if(buttonClicked != intFail){
                    //    char buffer[100];
                    //    sprintf(buffer, "Left click event at (%d, %d) hit button id#%d\n", mouseX, mouseY,buttonClicked);
                    //    logMessage(debug, buffer);
                    //}
                    //else{
                        char buffer[100];
                        sprintf(buffer, "Left click event at (%d, %d)\n", mouseX, mouseY);
                        logMessage(debug, buffer);
                    //}

                    // if buttonClicked getValue
                    if(currentScene == mainmenu){
                        // if(buttonClicked == getValue("playbtn")){
                        //     loadScene(game);
                        // }
                        // else if(buttonClicked == getValue("settingsbtn")){
                        //     loadScene(settings);
                        // }
                        // else if(buttonClicked == getValue("quitbtn")){
                        //     quit = true;
                        // }
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

    // free shit
    free(smallButton);
    freeTrackedObjects();

    shutdownSaveData();

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