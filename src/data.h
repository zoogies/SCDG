#ifndef DATA_H
#define DATA_H

#include <jansson.h>

typedef struct {
    char* key;
    int value;
} KeyValuePair;

void addObject(const char* key, int value);

json_t *initSaveData(char *path);

json_t *getObject(json_t *parent, char *key);

int getValue(const char* key);

int getInteger(json_t *parent, char *key);

int getArrayInt(json_t *parent, int index);

void writeInt(json_t *parent, char *keyName, int toWrite);

void shutdownSaveData();

void freeTrackedObjects();

#endif