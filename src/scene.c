#include <string.h>
#include <stdbool.h>

#include <jansson.h>

#include "game.h"
#include "data.h"
#include "scene.h"
#include "state.h"
#include "callbacks.h"

#include "engine/logging.h"
#include "engine/graphics.h"
#include "engine/audio.h"

// declare some globals
json_t *SCENEEVENTS = NULL;
int currentEventIndex = 0;
char* currentScene = NULL;

/*
    Returns an integer channel number from a string key,
    looks up in the global gamedata_keys object

    TODO: should this go into another file? utils.c?
*/
int getChannelByKeyName(char *key){
    return getInteger(getObject(gamedata_keys,"channel"),key);
}

/*
    Advances the scene to the next event

    TODO:
    - pause, play sound, misc events
    - some predefined behavior when scene ends (stopper event?) default case event too
    - last scene played or some savedata mechanism in savedata

    THOUGHTS:
    - maybe this is something that should just be passed to lua ngl
       - would involve a lot of creating new lua interface methods though, unless we have lua
         pipeline commands through one C interface function which dispatches them accordingly
*/
void advanceScene(){
    if(SCENEEVENTS != NULL){
        json_t *event = getArrayIndex(SCENEEVENTS,currentEventIndex + 1); // TODO: only increment if valid event

        if(event != NULL){
            if(strcmp(getString(event, "type"),"dialog") == 0){
                // update our speaker text
                updateText(getState(stateCollection, "speaker name")->intValue, getString(event, "speaker"));
                updateText(getState(stateCollection, "speaker text")->intValue, getString(event, "text"));

                // update our character image
                // TODO: segfault should have error catching for getstate failures
                int id = getState(stateCollection, "speaker image")->intValue;
                updateImage(id, getString(event, "speaker src"));

                // play dialog sfx
                playSound(getString(event,"speaker sfx"), getChannelByKeyName(getString(event,"track")), 0);
            }
            currentEventIndex++;
        }
    }

    return;
}

/*
    take in keys, prototypes and renderObjects array and construct the scene
*/
void constructScene(json_t *pObjects, json_t *keys, json_t *prototypes){
    json_t *depthKeys = getObject(keys,"depth");
    json_t *fontKeys = getObject(keys,"font");

    json_t *TMP = NULL;  // Declare a temporary JSON object
    
    // loop through all renderObjects
    for(size_t i = 0; i<json_array_size(pObjects); i++){
        json_t *obj = getArrayIndex(pObjects,i);

        // test for prototype field and construct if existant (overwrites obj)
        char *prototypeName = getStringNOWARN(obj,"prototype");
        if(prototypeName != NULL){
            json_t* prototype = getObject(prototypes, prototypeName);
            TMP = mergeJSON(obj, prototype);
            
            // replace obj with tmp
            obj = TMP;
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
            created = -1;
        }

        // check if our object has a kvp so we can be tracking it
        char *identifier = getStringNOWARN(obj,"identifier");
        if(identifier != NULL){
            // add to kvp (or update?)
            char buffer[100];
            snprintf(buffer, sizeof(buffer),  "Adding '%s' to tracked objects.\n", identifier);
            logMessage(debug, buffer);
            if(created > 0){ // NOTE: this will cause issues if this function is used to create engine objects
                addState(stateCollection, identifier, (State){.type = STATE_INT, .intValue = created});
            }
        }
        // if we created a new json with a refcount for tmp we need to decref it so it deletes and reset it
        if(TMP != NULL){
            json_decref(TMP);
            TMP = NULL;
        }
    }
}

/*
    Handles playing the music for a scene on start
*/
void startSceneMusic(json_t *scene,json_t *keys){
    json_t *channelKeys = getObject(keys,"channel");
    json_t *pMusic = getObject(scene,"music");
    playSound(getString(pMusic,"src"),getInteger(channelKeys, getString(pMusic, "channel")),getInteger(pMusic, "loops"));
}

/*
    takes in a scene name and sets up scene
    TODO: could we pop out old renderlist to another thread and free it seperately to speed up construction?
*/
void loadScene(char* scene){
    /*
        duplicate our parameter to avoid accessing freed memory
        and set the global scene value to the current scene
    */
    if(currentScene != NULL){
        free(currentScene);
    }
    scene = currentScene = strdup(scene); // TODO: fix this memory leak

    // we are going to start a counter to see how long the scene takes to load
    Uint32 startTime = SDL_GetTicks(); // get the current time (we will use this also to calculate playtime)

    updatePlaytime(startTime);

    // clear all game objects to prep for switching scenes
    clearAll(false);

    // clear our state TODO: should (some) state persist?
    clearStateCollection(stateCollection);

    // load keys and json scenes dict
    json_t *scenes = getObject(GAMEDATA,"scenes");

    json_t *_scene = getObject(scenes,scene);

    json_t *pObjects;

    logMessage(debug, "Loading a scene.\n");

    startSceneMusic(_scene,gamedata_keys);
    
    // get our scene objects and render them all
    pObjects = getArray(_scene,"renderObjects");

    char *prototypeName = getStringNOWARN(_scene,"prototype");
    json_t *TMP = NULL;
    if(prototypeName != NULL){
        json_t* prototype = getObject(getObject(gamedata_scene_prototypes, prototypeName),"renderObjects");
        TMP = mergeJSON(pObjects,prototype);
        pObjects = TMP;
    }

    constructScene(pObjects,gamedata_keys,gamedata_prototypes);

    if(TMP != NULL){
        json_decref(TMP);
    }
    
    // catch special scenes with additional setup
    if(strcmp(scene,"settings") == 0){
        // update our volume text
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "%d%%",(int)((float) VOLUME / 128 * 100));
        int id = getState(stateCollection, "volume-text")->intValue;
        updateText(id,buffer);
    }

    // set our scene events (copying it so we can decref GAMEDATA after)
    teardownScene(); // make sure we free our old scene events
    json_t *pEvents = getObjectNOWARN(_scene,"events");
    if(pEvents != NULL){
        SCENEEVENTS = json_deep_copy(pEvents);
    }
    
    Uint32 endTime = SDL_GetTicks();

    // Calculate the elapsed time
    Uint32 elapsedTime = endTime - startTime;

    // Print the elapsed time in milliseconds
    char buffer[100];
    snprintf(buffer, sizeof(buffer),  "Scene loaded in %ums.\n", elapsedTime);
    logMessage(debug, buffer);
    
    // run our first event
    advanceScene();
}

void setupSceneManager(){
    // populate our global game data variables
    gamedata_keys = getObject(GAMEDATA,"keys");
    gamedata_prototypes = getObject(GAMEDATA,"prototypes");
    gamedata_scene_prototypes = getObject(GAMEDATA,"scene prototypes");
}

void teardownScene(){
    // free our scene events
    if (SCENEEVENTS) {
        json_decref(SCENEEVENTS);
        SCENEEVENTS = NULL;
    }
    currentEventIndex = 0;
}

void shutdownSceneManager(){
    teardownScene();
    free(currentScene);
}