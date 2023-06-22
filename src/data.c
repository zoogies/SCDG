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

/////////////////////////// LINKED LIST FUNCS ////////////////////////////////

void addItem(LinkedList* list, const char* key, void* value, Type type) {
    Node* newNode = malloc(sizeof(Node));
    newNode->key = malloc((strlen(key) + 1) * sizeof(char));
    strcpy(newNode->key, key);
    newNode->type = type;
    newNode->next = NULL;

    switch (type) {
        case INT:
            newNode->value.intValue = *(int*)value;
            break;
        case SDL_COLOR:
            newNode->value.colorValue = *(SDL_Color*)value;
            break;
        case TTF_FONT:
            newNode->value.fontValue = *(TTF_Font**)value;
            break;
    }

    if (list->tail == NULL) {
        list->head = newNode;
        list->tail = newNode;
    } else {
        list->tail->next = newNode;
        list->tail = newNode;
    }
}

Node* getItem(LinkedList* list, const char* key) {
    Node* currentNode = list->head;
    while (currentNode != NULL) {
        if (strcmp(currentNode->key, key) == 0) {
            return currentNode;
        }
        currentNode = currentNode->next;
    }
    return NULL;
}

void freeLinkedList(LinkedList* list) {
    Node* currentNode = list->head;
    while (currentNode != NULL) {
        Node* nextNode = currentNode->next;
        free(currentNode->key);

        if (currentNode->type == TTF_FONT) {
            TTF_CloseFont(currentNode->value.fontValue);
        }

        free(currentNode);
        currentNode = nextNode;
    }
    logMessage(debug, "freed linked list\n");
}

/////////////////////////// JSON FUNCTIONS ////////////////////////////////

// load a json file and return its json_t, null if not existant or could not be accessed
json_t *loadJSONFile(char *path){
    if(access(getPathStatic(path), F_OK) == -1) {
        char buffer[100];
        sprintf(buffer, "Could not access file '%s'.\n", path);
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

// load save data, create it not existant with sensible defaults
json_t *getSaveData(char *path) {
    json_t *saveData = loadJSONFile(path);
    if(saveData == NULL) {
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
            FILE *pFile = fopen(getPathStatic(path), "w");
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

    // open save data
    json_error_t err;
    json_t *pRoot = json_load_file(getPathStatic(path), 0, &err);
    if (!pRoot) {
        logMessage(error, "Error parsing JSON file.\n");
    }
    return pRoot;
}

json_t *getGameData(char *path){
    json_error_t err;
    json_t *pRoot = json_load_file(getPathStatic(path), 0, &err);
    if (!pRoot) {
        logMessage(error, "Error parsing JSON file.\n");
    }
    return pRoot;
}

json_t *getObject(json_t *parent, char *key){
    json_t *pObject = json_object_get(parent, key);
    if (!pObject) {
        if(strcmp(key, "prototype") != 0 && strcmp(key, "identifier") != 0){
            char buffer[100];
            sprintf(buffer, "Error parsing JSON file for '%s'.\n", key);
            logMessage(error, buffer);
        }
        return NULL;
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

bool getBool(json_t *parent, char *key){
    json_t *pObject = getObject(parent, key);
    if (!pObject || !json_is_boolean(pObject)) {
        json_decref(pObject);
        exit(1); // TODO temp fix
    }
    return json_boolean_value(pObject);
}

float getFloat(json_t* parent, const char* field_name) {
    float value = 0.0;
    json_t* field = json_object_get(parent, field_name);

    if (json_is_number(field)) {
        value = json_number_value(field);
    }

    return value;
}

char *getString(json_t *parent, char *key){
    json_t *pObject = getObject(parent, key);
    if (!pObject || !json_is_string(pObject)) {
        json_decref(pObject);
        return NULL; // TODO SIMPLIFY FIX EVERYTHING
        // FIXME all return null for none handle error outside this file
    }
    return (char*)json_string_value(pObject);
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

// array getters

int getArrayInt(json_t *parent, int index){
    json_t *pObject = getArrayIndex(parent, index);
    if (!pObject || !json_is_integer(pObject)) {
        json_decref(pObject);
        logMessage(error,"FAILED GETTING ARRAY INT BY INDEX\n");
    }
    return json_integer_value(pObject);
}

char *getArrayString(json_t *parent, int index){
    json_t *pObject = getArrayIndex(parent, index);
    if (!pObject || !json_is_string(pObject)) {
        json_decref(pObject);
        logMessage(error,"FAILED GETTING ARRAY STRING BY INDEX\n");
    }
    return (char*)json_string_value(pObject);
}

void dumpJSON(json_t *parent){
    json_dumpf(parent, stdout, JSON_INDENT(2));
    printf("\n");
}

// modification values

// update our json at path with passed json_t data
void saveJSONFile(json_t *data, char *path){
    json_dump_file(data, getPathStatic(path), JSON_INDENT(2));
}

json_t *writeInt(json_t *parent, char *keyName, int toWrite){
    json_t *newVal = json_integer(toWrite);
    json_object_set(parent, keyName, newVal);
    return newVal;
}