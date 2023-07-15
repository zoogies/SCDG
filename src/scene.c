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

/*
    take in keys, prototypes and renderObjects array and construct the scene
*/
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

        // if we created a new json with a refcount for tmp we need to decref it so it deletes
        if (tmp != NULL) {
            json_decref(tmp);
        }
    }
}

/*
    convert a scene name into its cooresponding enum
    TODO: this will get really long in the future
*/
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
    takes in a scene enum and setup scene
    TODO: could we pop out old renderlist to another thread and free it seperately to speed up construction?
*/
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