#include <stdio.h>
#include <jansson.h>

/*
    What we want to test:
    - refcount with different operations

    Things we need jansson to do:
    - load our json files and let us read/modify fields
    - pass references (pointers) to json_t objects

    Thoughts:
    - its possible that we make only game code use jansson because having engine
    depend on it is kindof stupid but this adds a lot more work that will never be used
*/

char *path = "./test.json";

json_t *loadJSONFile(char *path){
    json_error_t err;
    json_t *pRoot = json_load_file(path, 0, &err);
    if (!pRoot) {
        printf("Error parsing JSON file.\n");
    }
    return pRoot;
}

struct dummyStruct {
    json_t *data;
};

int getDataInt(struct dummyStruct ds){
    json_t *field = json_object_get(ds.data, "number");
    int ret = json_integer_value(field);
    // json_decref(field); NOTE: THIS IS A DOUBLE FREE BECAUSE WE ARE TRYING TO FREE A POINTER ASSIGNED TO A STRUCT FOR AN OBJECT WHICH IS USED LATER
    return ret;
}

int main() {
    json_t *root = loadJSONFile(path);
    json_dumpf(root, stdout, JSON_INDENT(2));
    struct dummyStruct ds;
    ds.data = root;
    printf("num:%d\n", getDataInt(ds));
    json_decref(root);
    return 0;
}

/*
    Thoughts p2:
    - maybe we can keep our game data reading stuff in its own file with its own globals, that way we arent tracking and throwing pointers around from main

    Solutions:
    - step through game code and look at the refcount of EVERYTHING as it gets created
      (or wrap decref again and print the refcount before and after)... identify the source of the overflow (which is likely from a double free)

    Steps:
    - verify with GPT everything looks normal
    - step through game code and identify any strange frees or operations
    - rewrite game jansson code to be normal and working properly
        - might entail some kind of global kvp lookup so not working with locally scoped structs and freeing them over and over
*/

/*
    SOLUTION:
    - we only need to decref on functions that return new references NOT borrowed references
    -> go through the code, destroy all pointless decrefs
       - maybe rewrite our jansson interface functions in data.c to keep top level borrowed refs to the root of our needed files and thats it
*/

