#ifndef ANIMATION_H
#define ANIMATION_H

#include <SDL2/SDL.h>

struct animation {
    int framerate;
    int lastUpdate;
    SDL_Texture* frames[100];
};

void updateTrackedAnimations();

void registerAnimatedObject();

#endif