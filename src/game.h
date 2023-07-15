#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#include <jansson.h>

#include "state.h"

// globals
extern bool gamedebug;
extern bool quit;
extern enum scenes currentScene;
extern StateCollection* stateCollection;
extern int VOLUME;

TTF_Font *useFont(char *key);

SDL_Color *useColor(char *key, json_t *keys);

void updatePlaytime(Uint32 startTime);

void volumeUp();

void volumeDown();

void updateGameScreenSize();

int shutdownGame();

// main entry point
int main();

#endif