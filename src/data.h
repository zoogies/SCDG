#ifndef DATA_H
#define DATA_H

#include "engine/graphics.h"

#include <jansson.h>

typedef enum ValueType {
    TYPE_COLOR,
    TYPE_FONT,
    TYPE_INT
} ValueType;

typedef struct Value {
    ValueType type;
    void* data;
} Value;

typedef struct Node {
    char key[20];
    Value value;
    struct Node* next;
} Node;

typedef struct LinkedList {
    Node* head;
} LinkedList;

typedef struct TypedLinkedList {
    LinkedList list;
    ValueType type;
} TypedLinkedList;

LinkedList* createLinkedList();

Node* createItem(char* key, ValueType type, void* value_ptr);

void addItem(LinkedList* list, Node* new_node);

void* getItem(LinkedList* list, char* key);

void *getTypedItem(TypedLinkedList *tlist, char *key);

void freeNodes(LinkedList* list);

void freeLinkedList(LinkedList* list);

json_t *loadJSONFile(char *path);

json_t *getSaveData(char *path);

json_t *getGameData(char *path);

json_t *getObject(json_t *parent, char *key);

int getInteger(json_t *parent, char *key);

bool getBool(json_t *parent, char *key);

float getFloat(json_t* parent, const char* field_name);

char *getString(json_t *parent, char *key);

json_t *getArray(json_t *parent, char *key);

json_t *getArrayIndex(json_t *parent, int index);

int getArrayInt(json_t *parent, int index);

char *getArrayString(json_t *parent, int index);

void dumpJSON(json_t *parent);

void saveJSONFile(json_t *data, char *path);

json_t *writeInt(json_t *parent, char *keyName, int toWrite);

#endif