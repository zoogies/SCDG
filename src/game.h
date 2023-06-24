#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

// initialize a global debug flag at false
extern bool gamedebug;
extern bool quit;

enum scenes {
    mainmenu = 0,
    game = 1,
    settings = 2
};

void loadScene(enum scenes scene);

enum scenes getSceneNameEnum(char *name);

// main entry point
int main();

#endif