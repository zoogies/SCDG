#ifndef DATA_H
#define DATA_H

typedef struct {
    char* key;
    int value;
} KeyValuePair;

void addObject(const char* key, int value);

int getValue(const char* key);

void freeTrackedObjects();

#endif