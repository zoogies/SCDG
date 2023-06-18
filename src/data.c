// function for working with game data. keeps track of global state and works with reading and writing to files

#include "data.h"
#include "engine/logging.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

KeyValuePair* trackedObjects = NULL;
int trackedObjectsCount = 0;
int trackedObjectsCapacity = 0;

void addObject(const char* key, int value) {
    char buffer[100];
    sprintf(buffer, "adding key '%s' with value %d", key, value);
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
    printf("am i segfaulting here?\n");
    for (int i = 0; i < trackedObjectsCount; i++) {
        free(trackedObjects[i].key);
    }
    free(trackedObjects);
    trackedObjects = NULL;
    trackedObjectsCount = 0;
    trackedObjectsCapacity = 0;
    printf("nope\n");
}