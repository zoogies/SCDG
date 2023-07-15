#ifndef SCENE_H
#define SCENE_H

enum scenes {
    mainmenu = 0,
    game = 1,
    settings = 2
};

void loadScene(enum scenes scene);

enum scenes getSceneNameEnum(char *name);

#endif