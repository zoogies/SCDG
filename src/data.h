#ifndef DATA_H
#define DATA_H

#include "engine/graphics.h"

#include <jansson.h>

typedef enum {
    INT,
    SDL_COLOR,
    TTF_FONT
} Type;

typedef struct Node {
    char* key;
    Type type;
    union {
        int intValue;
        SDL_Color colorValue;
        TTF_Font* fontValue;
    } value;
    struct Node* next;
} Node;

typedef struct {
    Node* head;
    Node* tail;
} LinkedList;

void addItem(LinkedList* list, const char* key, void* value, Type type);

Node* getItem(LinkedList* list, const char* key);

json_t *initSaveData(char *path);

json_t *getGameData(char *path);

json_t *getObject(json_t *parent, char *key);

int getValue(const char* key);

int getInteger(json_t *parent, char *key);

bool getBool(json_t *parent, char *key);

float getFloat(json_t* parent, const char* field_name);

char *getString(json_t *parent, char *key);

json_t *getArray(json_t *parent, char *key);

int getArrayInt(json_t *parent, int index);

char *getArrayString(json_t *parent, int index);

json_t *getArrayIndex(json_t *parent, int index);

void dumpJSON(json_t *parent);

void writeInt(json_t *parent, char *keyName, int toWrite);

void shutdownSaveData();

void freeLinkedList(LinkedList* list);

#endif