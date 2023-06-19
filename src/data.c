// function for working with game data. keeps track of global state and works with reading and writing to files

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <jansson.h>

#include "data.h"
#include "engine/engine.h"
#include "engine/logging.h"

/////////////////////////// KVP STUFF ////////////////////////////////

KeyValuePair* trackedObjects = NULL;
int trackedObjectsCount = 0;
int trackedObjectsCapacity = 0;

void addObject(const char* key, int value) {
    char buffer[100];
    sprintf(buffer, "adding key '%s' with value %d\n", key, value);
    logMessage(debug, buffer);
    // Check if capacity is sufficient, otherwise reallocate memory
    if (trackedObjectsCount >= trackedObjectsCapacity) {
        // Double the capacity or use a different strategy to resize the array
        trackedObjectsCapacity = (trackedObjectsCapacity == 0) ? 1 : (trackedObjectsCapacity * 2);

        trackedObjects = realloc(trackedObjects, trackedObjectsCapacity * sizeof(KeyValuePair));
    }

    // Allocate memory for the key and copy its contents
    trackedObjects[trackedObjectsCount].key = malloc((strlen(key) + 1) * sizeof(char));
    strcpy(trackedObjects[trackedObjectsCount].key, key);
    trackedObjects[trackedObjectsCount].value = value;
    trackedObjectsCount++;
}

int getValue(const char* key) {
    // Search for the object in the trackedObjects array based on the key
    for (int i = 0; i < trackedObjectsCount; i++) {
        if (strcmp(trackedObjects[i].key, key) == 0) {
            return trackedObjects[i].value;
        }
    }

    // Return a default value if the key is not found
    return -1;
}

void freeTrackedObjects() {
    for (int i = 0; i < trackedObjectsCount; i++) {
        char buffer[100];
        sprintf(buffer, "freeing '%s'\n", trackedObjects[i].key);
        logMessage(debug, buffer);
        free(trackedObjects[i].key);
    }
    free(trackedObjects);
    trackedObjects = NULL;
    trackedObjectsCount = 0;
    trackedObjectsCapacity = 0;
}

/////////////////////////// JSON STUFF ////////////////////////////////

// NOTE TODO:
// certain really common field values should be assumed if they do not exist (tracked false by defualt)

json_t *pRoot = NULL;
char *savePath = NULL;

// check if save data exists and create it if not
// Check if the file exists
json_t *initSaveData(char *path) {
    if(access(path, F_OK) == -1) {
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
            FILE *pFile = fopen(getPathStatic("data/savedata.json"), "w");
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
    pRoot = json_load_file(getPathStatic("data/savedata.json"), 0, &err);
    if (!pRoot) {
        logMessage(error, "Error parsing JSON file.\n");
    }
    savePath = path;
    return pRoot;
}

json_t *getObject(json_t *parent, char *key){
    json_t *pObject = json_object_get(parent, key);
    if (!pObject) {
        char buffer[100];
        sprintf(buffer, "Error parsing JSON file for '%s'.\n", key);
        logMessage(error, buffer);
    }
    return pObject;
}

int getInteger(json_t *parent, char *key){
    json_t *pObject = getObject(parent, key);
    if (!pObject || !json_is_integer(pObject)) {
        json_decref(pObject);
        exit(1); // TODO temp fix
    }
    return json_integer_value(pObject);
}

json_t *getArray(json_t *parent, char *key){
    json_t *pObject = getObject(parent, key);
    if (!pObject || !json_is_array(pObject)) {
        json_decref(pObject);
        exit(1); // TODO temp fix
    }
    return pObject;
}

json_t *getArrayIndex(json_t *parent, int index){
    json_t *pObject = json_array_get(parent, index);
    if (!pObject) {
        char buffer[100];
        sprintf(buffer, "Error getting json array index %d.\n", index);
        logMessage(error, buffer);
    }
    return pObject;
}

int getArrayInt(json_t *parent, int index){
    json_t *pObject = getArrayIndex(parent, index);
    if (!pObject || !json_is_integer(pObject)) {
        json_decref(pObject);
        exit(1); // TODO temp fix
    }
    return json_integer_value(pObject);
}

// modification values

void syncChanges(){
    json_dump_file(pRoot, savePath, JSON_INDENT(2));
}

void writeInt(json_t *parent, char *keyName, int toWrite){
    json_t *newVal = json_integer(toWrite);
    json_object_set(parent, keyName, newVal);
    syncChanges();
}

void shutdownSaveData(){
    json_decref(pRoot);
}

// iterates through a scene and sets it up
// void setupScene(){
    
// }