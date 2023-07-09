#include "engine/uthash/uthash.h"
#include "state.h"
#include <stdlib.h>
#include <string.h>

StateCollection* createStateCollection() {
    StateCollection* collection = malloc(sizeof(StateCollection));
    collection->map = NULL;
    return collection;
}

void destroyStateCollection(StateCollection* collection) {
    StateValuePair* pair, *tmp;

    HASH_ITER(hh, collection->map, pair, tmp) {
        HASH_DEL(collection->map, pair);
        clearState(&pair->value);
        free(pair->key);
        free(pair);
    }

    free(collection);
}

void clearStateCollection(StateCollection* collection) {
    StateValuePair* pair, *tmp;

    HASH_ITER(hh, collection->map, pair, tmp) {
        HASH_DEL(collection->map, pair);
        clearState(&pair->value);
        free(pair->key);
        free(pair);
    }
}

void addState(StateCollection* collection, char* key, State state) {
    StateValuePair* pair = malloc(sizeof(StateValuePair));
    pair->key = strdup(key);
    pair->value = state;
    HASH_ADD_KEYPTR(hh, collection->map, pair->key, strlen(pair->key), pair);
}

State* getState(StateCollection* collection, char* key) {
    StateValuePair* pair;
    HASH_FIND_STR(collection->map, key, pair);
    if (pair != NULL)
        return &pair->value;
    return NULL;
}

void clearState(State* state) {
    if (state->type == STATE_STRING) {
        free(state->stringValue);
    }
}