#include "game.h"
#include "engine/logging.h"
#include "engine/graphics.h"
#include "engine/engine.h"
#include "data.h"

// NOTE: when we change a setting like this, its annoying to go through our LL
// and update screen points, so lets just reload settings

// NOTE/WARN: only save settings if the user successfully exits the settings menu

// struct all fields null, update fields that arent?

bool changed_windowMode = false;
bool changed_fpsCap = false;

char *new_windowMode = "";
int new_fpsCap = 0;

void actionHandler(struct callbackData *_data){
    json_t *data = _data->pJson;
    if(strcmp(getString(data, "action"), "quit") == 0){
        exit(shutdownGame());
    }
    else if(strcmp(getString(data, "action"), "changeWindowMode") == 0){
        char *mode = getString(data,"mode");
        if(strcmp(mode,"windowed") == 0){
            changeWindowMode(SDL_FALSE);
            new_windowMode = "windowed";
        }
        else if(strcmp(mode,"fullscreen") == 0){
            changeWindowMode(SDL_WINDOW_FULLSCREEN_DESKTOP);
            new_windowMode = "fullscreen";
        }
        else{
            logMessage(warning, "Invalid window mode, defaulting to windowed.\n");
            changeWindowMode(SDL_FALSE);
            loadScene(settings);
            return;
        }
        changed_windowMode = true;
        updateGameScreenSize();
        loadScene(settings);
    }
    else if(strcmp(getString(data, "action"), "changeFPS") == 0){
        int cap = getInteger(data, "fps");
        changeFPS(cap);
        new_fpsCap = cap;
        changed_fpsCap = true;
        loadScene(settings);
    }
}

void callbackHandler(struct callbackData *data){
    // printf("TYPE: %s\n",data->callbackType);
    // printf("DATA:\n");
    // dumpJSON(data->pJson);
    if(strcmp(data->callbackType, "loadscene") == 0){

        // -------------------------------------------------------------------------

        // TODO FIXME URGENT:
        // we should only update our save data settings 
        // if the user clicks the exit button when in the settings menu
        // we are waiting for global state to fix this
        json_t *SAVEDATA = getSaveData("data/savedata.json");
        if(changed_fpsCap){
            writeInt(getObject(SAVEDATA,"settings"),"framecap",new_fpsCap);
        }
        else if(changed_windowMode){
            writeString(getObject(SAVEDATA,"settings"),"window mode",new_windowMode);

            // if we changed window mode we also changed resolution (for now)
            json_t *arr = getArray(getObject(SAVEDATA,"settings"),"resolution");
            struct ScreenSize size = getCurrentResolution();
            writeArrayInt(arr,0,size.width);
            writeArrayInt(arr,1,size.height);
        }
        saveJSONFile(SAVEDATA,"data/savedata.json");

        json_decref(SAVEDATA);
        changed_fpsCap = false;
        changed_windowMode = false;
        
        // -------------------------------------------------------------------------

        loadScene(getSceneNameEnum(getString(data->pJson, "scene")));
    }
    else if(strcmp(data->callbackType, "action") == 0){
        actionHandler(data);
    }
    else if(strcmp(data->callbackType, "test") == 0){
        printf("TEST CALLBACK RECIEVED AND EXECUTED\n");
    }
}