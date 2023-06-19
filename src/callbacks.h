#ifndef CALLBACKS_H
#define CALLBACKS_H

// typedef int (*ButtonCallback)(enum callbacks callback);

enum callbacks {
    gotoMainMenu,
    gotoSettings,
    quitGame
};

int callbackHandler(enum callbacks callback);

#endif