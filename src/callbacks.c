#include "game.h"
#include "engine/logging.h"

void gotoMainMenu(){
    logMessage(debug, "Going to main menu\n");
    loadScene(mainmenu);
}

void gotoSettings(){
    logMessage(debug, "Going to settings\n");
    loadScene(settings);
}

void quitGame(){
    logMessage(debug, "Going to settings\n");
    quit=true;
}