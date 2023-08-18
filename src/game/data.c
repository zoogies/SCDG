// function for working with game data. keeps track of global state and works with reading and writing to files

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#include <jansson.h>

#include "data.h"
#include "engine/engine.h"
#include "engine/logging.h"
#include "engine/graphics.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// globals for game data information
json_t *GAMEDATA;
json_t *gamedata_keys;
json_t *gamedata_prototypes;
json_t *gamedata_scene_prototypes;

json_t *SAVEDATA;

/*
    TODO:
    - json functions dont exit(1)
*/

/////////////////////////// JSON FUNCTIONS ////////////////////////////////

// Thoughts:
// - by the nature of tracking roots and refs, maybe this would benefit from some sort of global store

/*
    CRITICALLY IMPORANT NOTE:
    you only need to decref a json_t which ISNT borrowed. getting objects from other 
    objects returns borrowed refs which DO NOT need decref'd. The root of all the
    json_t objects is the one that needs to be decref'd

    you need to VERY CLEARLY notate what is a root json_t and what is just a borrowed value
*/


/*
    load a json file and return its json_t, null if not existant or could not be accessed
    returns a NEW reference refcount 1

    TODO: fix alleged memory leak.
    what we know:
    - does not happen unless you load intro
*/
json_t *loadJSONFile(char *path){
    if(access(getPathStatic(path), F_OK) == -1) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Could not access file '%s'.\n", path);
        logMessage(error, buffer);
        return NULL;
    }

    json_error_t err;
    json_t *pRoot = json_load_file(getPathStatic(path), 0, &err);
    if (!pRoot) {
        logMessage(error, "Error parsing JSON file.\n");
    }
    return pRoot;
}

/*
    This function loads save data, if it doesnt exist it will create it
    with sensible defaults. always returns a json_t pointer
*/
json_t *getSaveData(char *path) {
    json_t *saveData = loadJSONFile(path); // this is a pointer to a root json_t DO NOT DECREF THIS VAR DIRECTLY
    if(saveData == NULL) {
        logMessage(warning, "Save data not found, creating...\n");

        // we need to get the screen size to set the defualt resolution
        struct ScreenSize size = getScreenSize();

        // Create sensible defaults for the game data
        // creates new json_t with refcount 1
        saveData = json_pack("{s:{s:[i,i], s:s, s:i, s:i}s:{s:s,s:i}}",
                                        "settings",
                                        "resolution", size.width, size.height,
                                        "window mode", "fullscreen",
                                        "volume", 16,
                                        "framecap", -1,
                                        "user",
                                        "name","unknown",
                                        "playtime",0);

        // Save JSON object to file
        if (saveData != NULL) {
            FILE *pFile = fopen(getPathStatic(path), "w");
            if (pFile != NULL) {
                json_dumpf(saveData, pFile, JSON_INDENT(2));
                fclose(pFile);
            }
            else {
                logMessage(error, "Failed to open save data file.\n");
            }
        }
        else {
            logMessage(error,"failed to create JSON object.\n");
        }
    }
    else {
        logMessage(info, "Save data found, reading...\n");
    }

    // one way or another, saveData is now our save data so we can return it
    return saveData;
}

/*
    Will create our global data variables and load the game data
    and save data into them
*/
void initializeDataManager(){
    // load our game data
    GAMEDATA = loadJSONFile("data/gamedata.json");
    if(GAMEDATA == NULL){
        logMessage(error, "Could not load game data.\n");
        exit(1);
    }

    // load our save data
    SAVEDATA = getSaveData("data/savedata.json");
    if(SAVEDATA == NULL){
        logMessage(error, "Could not load save data.\n");
        exit(1);
    }

    // load our keys and prototypes
    gamedata_keys = getObject(GAMEDATA,"keys");
    gamedata_prototypes = getObject(GAMEDATA,"prototypes");
    gamedata_scene_prototypes = getObject(GAMEDATA,"scene prototypes");
    logMessage(debug, "Initialized data manager.\n");
}

void shutdownDataManager(){
    // decref our global json_t objects
    json_decref(GAMEDATA);
    GAMEDATA = NULL;
    json_decref(SAVEDATA);
    SAVEDATA = NULL;
    logMessage(debug, "Shutdown data manager.\n");
}

// JSON FIELD SPECIFIC ACCESSOR FUNCTIONS: (ALL RETURN BORROWED REFS)

json_t *getObject(json_t *parent, char *key) {
    json_t *pObject = json_object_get(parent, key);

    if (pObject == NULL) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Error parsing JSON file for '%s'.\n", key);
        logMessage(error, buffer);
        return NULL;
    }

    return pObject;
}

json_t *getObjectNOWARN(json_t *parent, char *key) {
    json_t *pObject = json_object_get(parent, key);

    if (pObject == NULL) {
        return NULL;
    }

    return pObject;
}

int getInteger(json_t *parent, char *key){
    json_t *pObject = getObject(parent, key);
    if (!pObject || !json_is_integer(pObject)) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Key '%s' was not expected type of integer.\n", key);
        logMessage(error, buffer);
        exit(1);
    }
    return json_integer_value(pObject);
}

bool getBool(json_t *parent, char *key){
    json_t *pObject = getObject(parent, key);
    if (!pObject || !json_is_boolean(pObject)) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Key '%s' was not expected type of boolean.\n", key);
        logMessage(error, buffer);
        exit(1);
    }
    return json_boolean_value(pObject);
}

// returns false if theres an error fetching it. will this cause any issues?
bool getBoolNOWARN(json_t *parent, char *key){
    json_t *pObject = getObjectNOWARN(parent, key);
    if (!pObject || !json_is_boolean(pObject)) {
        return false;
    }
    return json_boolean_value(pObject);
}

float getFloat(json_t* parent, char* key) {
    float value = 0.0;
    json_t* field = json_object_get(parent, key);

    if (json_is_number(field)) {
        value = json_number_value(field);
    }
    else{
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Key '%s' encountered error converting to float.\n", key);
        logMessage(error, buffer);
    }

    return value;
}

char *getString(json_t *parent, char *key){
    json_t *pObject = getObject(parent, key);
    if (!pObject || !json_is_string(pObject)) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Key '%s' was not expected type of string.\n", key);
        logMessage(error, buffer);
        return NULL;
    }
    return (char*)json_string_value(pObject);
}

char *getStringNOWARN(json_t *parent, char *key){
    json_t *pObject = getObjectNOWARN(parent, key);
    if (!pObject || !json_is_string(pObject)) {
        return NULL;
    }
    return (char*)json_string_value(pObject);
}

json_t *getArray(json_t *parent, char *key){
    json_t *pObject = getObject(parent, key);
    if (!pObject || !json_is_array(pObject)) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Key '%s' was not expected type of array.\n", key);
        logMessage(error, buffer);
        return NULL;
    }
    return pObject;
}

json_t *getArrayIndex(json_t *parent, int index){
    json_t *pObject = json_array_get(parent, index);
    if (!pObject) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Error getting json array index '%d'.\n", index);
        logMessage(error, buffer);
        return NULL;
    }
    return pObject;
}

// array getters

int getArrayInt(json_t *parent, int index){
    json_t *pObject = getArrayIndex(parent, index);
    if (!pObject || !json_is_integer(pObject)) {
        logMessage(error,"FAILED GETTING ARRAY INT BY INDEX\n");
        exit(1); // TODO FIXME
    }
    return json_integer_value(pObject);
}

char *getArrayString(json_t *parent, int index){
    json_t *pObject = getArrayIndex(parent, index);
    if (!pObject || !json_is_string(pObject)) {
        logMessage(error,"FAILED GETTING ARRAY STRING BY INDEX\n");
        return NULL;
    }
    return (char*)json_string_value(pObject);
}

// debugging function to print json to stdout
void dumpJSON(json_t *parent){
    json_dumpf(parent, stdout, JSON_INDENT(2));
    printf("\n");
}

// merge two json objects (or lists) into one (make sure to decref the result)
json_t* mergeJSON(json_t* obj, json_t* prototype) {
    if (json_is_array(obj) && json_is_array(prototype)) {
        json_t* mergedArray = json_array();

        // Merge objects from obj
        size_t index;
        json_t* value;
        json_array_foreach(obj, index, value) {
            json_array_append(mergedArray, json_incref(value));
        }

        // Merge objects from prototype
        json_array_foreach(prototype, index, value) {
            json_array_append(mergedArray, json_incref(value));
        }

        return mergedArray;
    }
    else {
        // Merge JSON objects
        const char* key;
        json_t* value;
        
        json_t* mergedObject = json_object();
        
        // iterate through prototype and add all fields that don't exist in obj
        json_object_foreach(prototype, key, value) {
            if (!json_object_get(obj, key)) {
                json_object_set(mergedObject, key, value);
            }
        }
        
        // iterate through obj and add all fields
        json_object_foreach(obj, key, value) {
            json_object_set(mergedObject, key, value);
        }
        
        return mergedObject;
    }
}

// ---------------------------------------------------------------------------------------
// modification functions
// ---------------------------------------------------------------------------------------

// update our json at path with passed json_t data
void saveJSONFile(json_t *data, char *path){
    json_dump_file(data, getPathStatic(path), JSON_INDENT(2));
}

// modified the passed parent and adds an int at the keyname
void writeInt(json_t *parent, char *keyName, int toWrite){
    json_t *newVal = json_integer(toWrite);
    json_object_set(parent, keyName, newVal);
    json_decref(newVal);
}

void writeArrayInt(json_t *parent, size_t index, int toWrite){
    json_t *newVal = json_integer(toWrite);
    json_array_set(parent, index, newVal);
    json_decref(newVal);
}

void writeString(json_t *parent, char *keyName, char *toWrite){
    json_t *newVal = json_string(toWrite);
    json_object_set(parent, keyName, newVal);
    json_decref(newVal);
}

/*
    EXTREMELY IMPORTANT CRITICAL THOUGHT:
    when we do our callback, this needs to be a NEW json_t object that
    has its own refcount at 1 which is decref'd on button destroy only (lonely HAHA get it?)
*/

/*
    NOTE:
    ANY NEW REFERENCE OR ROOT JSON_T REFERENCE IS WRITTEN IN CAPS
    json_t *root = getSaveData("data/save.json");
    ANY BORROWED REFERENCE IS WRITTEN IN LOWERCASE
    json_t *pObject = getObject(root, "settings");
*/