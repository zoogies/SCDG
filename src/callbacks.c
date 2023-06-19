#include "game.h"
#include "callbacks.h"
#include "engine/logging.h"

int callbackHandler(enum callbacks callback){
    switch(callback){
        case gotoMainMenu:
            logMessage(debug, "Going to main menu\n");
            loadScene(mainmenu);
            break;
        case gotoSettings:
            logMessage(debug, "Going to settings\n");
            loadScene(settings);
            break;
        case quitGame:
            logMessage(debug, "Quitting game\n");
            quit=true;
            break;
        default:
            logMessage(error, "Invalid callback\n");
            break;
    }
    return 0;
}

// recieve json_t "callback" field and look at its "type" field, return function pointer to callback function



// callbacktype = generic, loadscene
/*
    "callback":{
        "type":"loadscene",
        "scene":"mainmenu"
    }
*/

// 