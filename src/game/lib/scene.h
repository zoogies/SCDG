#ifndef SCENE_H
#define SCENE_H

// global scene tracking string
extern char* currentScene;

// TODO: MOVEME?
int getChannelByKeyName(char *key);

void advanceScene();

void loadScene(char* scene);

void setupSceneManager();

void teardownScene();

void shutdownSceneManager();

#endif