#ifndef STATE_H
#define STATE_H

#include "engine/uthash/uthash.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef enum {
    STATE_INT,
    STATE_FLOAT,
    STATE_STRING,
} StateType;

typedef struct {
    StateType type;
    union {
        int intValue;
        float floatValue;
        char* stringValue;
    };
} State;

typedef struct {
    char* key;
    State value;
    UT_hash_handle hh;
} StateValuePair;

typedef struct {
    StateValuePair* map;
} StateCollection;

StateCollection* createStateCollection();

void destroyStateCollection(StateCollection* collection);

void clearStateCollection(StateCollection* collection);

void addState(StateCollection* collection, char* key, State state);

State* getState(StateCollection* collection, char* key);

void clearState(State* state);

#endif