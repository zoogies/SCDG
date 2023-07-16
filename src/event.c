#include <string.h>

#include <SDL2/SDL.h>

#include "game.h"
#include "scene.h"

#include "engine/engine.h"
#include "engine/graphics.h"
#include "engine/logging.h"
#include "engine/audio.h"

// tracks whether the console is open or not
bool consoleOpen = false;
char consoleString[100];

/*
    Takes in an SDL_Event and handles it
*/
void handleEvent(SDL_Event e){
    switch (e.type) {
        case SDL_QUIT: {
            quit = true;
            break;
        }
        case SDL_MOUSEBUTTONDOWN: {
            if (e.button.button == SDL_BUTTON_LEFT) {
                int mouseX = e.button.x;
                int mouseY = e.button.y;
                checkClicked(mouseX, mouseY);
                
                char buffer[100];
                snprintf(buffer, sizeof(buffer), "Left click event at (%d, %d)\n", mouseX, mouseY);
                logMessage(debug, buffer);
            }
            break;
        }
        case SDL_KEYDOWN: {
            switch (e.key.keysym.sym) {
                case SDLK_BACKQUOTE: {
                    toggleOverlay();
                    break;
                }
                case SDLK_TAB: {
                    consoleString[0] = '>';
                    consoleString[1] = '\0';
                    toggleConsole();
                    consoleOpen = !consoleOpen;
                    break;
                }
                default: {
                    if (consoleOpen) {
                        if (e.key.keysym.sym != SDLK_RETURN) {
                            if (e.key.keysym.sym == SDLK_BACKSPACE) {
                                size_t bufferLength = strlen(consoleString);
                                if (bufferLength > 1) {
                                    consoleString[bufferLength - 1] = '\0';
                                    updateText(-902, consoleString);
                                }
                            } else {
                                if (strlen(consoleString) < 100 - 1) {
                                    strncat(consoleString, (char*)&e.key.keysym.sym, 1);
                                    updateText(-902, consoleString);
                                } else {
                                    logMessage(error, "Buffer at max length!\n");
                                    playSound("sfx/pipe.mp3", -1, 0);
                                }
                            }
                        } else {
                            char buffer[100];
                            snprintf(buffer, sizeof(buffer), "Recieved command: %s\n", consoleString);
                            logMessage(debug, buffer);
                            char* token = strtok(consoleString, " ");
                            if (token != NULL) {
                                if (strcmp(token, ">load") == 0) {
                                    token = strtok(NULL, " ");
                                    if (token != NULL) { // debug print current scene
                                        if(strcmp(token,"main") == 0){
                                            loadScene("main menu");
                                        }
                                        else{
                                            /* persist this string so it doesnt get destroyed by
                                            unknown otherworldly powers beyond my comprehension */
                                            char *scene = strdup(token);
                                            loadScene(scene);
                                        }
                                    }
                                } else if (strcmp(token, ">reload") == 0) {
                                    if (token != NULL) {
                                        loadScene(currentScene); // reload current scene
                                    }
                                }
                                else if (strcmp(token, ">quit") == 0) {
                                    if (token != NULL) {
                                        quit = true;
                                    }
                                } else {
                                    logMessage(error, "Invalid command!\n");
                                    playSound("sfx/pipe.mp3", -1, 0);
                                }
                            }
                            consoleString[0] = '>';
                            consoleString[1] = '\0';
                            updateText(-902, consoleString);
                        }
                    }
                    break;
                }
            }
            break;
        }
        case SDL_WINDOWEVENT: {
            if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                // Reset the viewport when the game window regains focus
                // setViewport(SCREEN_WIDTH, SCREEN_HEIGHT);
            }
            break;
        }
    }
}