/*
    TODO: GRAPHICS
    - return pointers to renderObject structs so game can modify them as they see fit
    - look into passing a pointer for SDL color instead of the actual struct as it should
      theoretically be faster
    - consider having images in engine start at their center, so game has to do no calculations for finding middles of things
    - some sort of system for re render only updated renderObjects
*/

#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "engine.h"
#include "graphics.h"

// define globals for file
SDL_Window *pWindow = NULL;
SDL_Surface *pScreenSurface = NULL;
SDL_Renderer *pRenderer = NULL;
renderObject *pRenderListHead = NULL;

// int that increments each renderObject created, allowing new unique id's to be assigned
int global_id = 0;

// NOTE: negative global_id's are reserved for engine components, 
// and traversing the list to clear renderObjects will only clear 
// non engine renderObjects

// fps related counters
int fpsUpdateTime = 0;
int frameCounter = 0;
int fps = 0;
int startTime = 0;
int fpscap = 0;
int desiredFrameTime = 0;  

// initialize some variables in engine to track screen size
float targetAspectRatio = 16.0f / 9.0f;
int virtualWidth = 1920;
int virtualHeight = 1080;
int xOffset = 0;
int yOffset = 0;

// constructor for render objects, invoked internally by createText() and createImage()
// NOTE: this function inserts highest depth objects at the front of the list
void addRenderObject(int identifier, renderObjectType type, int depth, float x, float y, float width, float height, SDL_Texture *pTexture, bool centered) {
    // debug output
    printf("\n\033[0;32mAdd\033[0;37m render object [\033[0;33mid %d\033[0;37m]\t\t",identifier);
    
    // translate our relative floats into actual screen coordinates for rendering
    int objX = (int)(x * (float)virtualWidth); // + xOffset;
    int objY = (int)(y * (float)virtualHeight); // + yOffset;
    int objWidth = (int)(width * (float)virtualWidth);
    int objHeight = (int)(height * (float)virtualHeight);

    // modify our coordinates if we want to render at its center
    if(centered){
        objX = objX - (objWidth / 2);
        objY = objY - (objHeight / 2);
    }

    // create bounding rect from parameters
    SDL_Rect rect = {objX, objY, objWidth, objHeight};

    // construct and malloc new object
    renderObject *pObj = (renderObject *)malloc(sizeof(renderObject));

    // self explanatory assignment of fields from parameters
    pObj->identifier = identifier;
    pObj->pTexture = pTexture;
    pObj->rect = rect;
    pObj->depth = depth;
    pObj->pNext = NULL;
    pObj->type = type;

    // if there are no renderObjects in the list, or the depth of this object is lower than the head
    if (pRenderListHead == NULL || pObj->depth < pRenderListHead->depth) {
        // make this object the head
        pObj->pNext = pRenderListHead; 
        pRenderListHead = pObj;
    } 
    else {
        // iterate through the renderObject queue starting at the head
        renderObject *pCurrent = pRenderListHead;

        // while there is a next item and the next items depth is less than 
        // our current objects depth, keep going
        while (pCurrent->pNext != NULL && pCurrent->pNext->depth < pObj->depth) {
            pCurrent = pCurrent->pNext;
        }

        // once we know where our object sits:
        pObj->pNext = pCurrent->pNext; // make our object point to the one after our current
        pCurrent->pNext = pObj; // make our current point to our object

        /*
            Object has been inserted inbetween current and next in order of depth

            object = [2]
            
            BEFORE:
            [0]->[1]->[3]

            AFTER:
            [0]->[1]->[2]->[3]
        */
    }
    
    // debug output
    debugOutputComplete();
    if(centered){
        printf("C: ");
    }
    else{
        printf("R: ");
    }
    printf("x:%d y:%d w:%d h:%d\n\n",objX,objY,objWidth,objHeight);
    // printf("HEAD ID=%d\n",pRenderListHead->identifier); debug output showing what ID head is
}

// remove a render object from the queue by its identifier
void removeRenderObject(int identifier) {
    // debug output
    printf("\033[0;31mRemove\033[0;37m render object [\033[0;33mid %d\033[0;37m]\t\t",identifier);
    
    // if our render list has zero items
    if (pRenderListHead == NULL) {
        printf("ERROR REMOVING RENDER OBJECT: HEAD IS NULL");
        return;
        // alarm and pass
    }

    // if the head is the item we are looking to remove
    if (pRenderListHead->identifier == identifier) {
        // pop our head into a new temp object
        renderObject *pToDelete = pRenderListHead;

        // set our head to the previous 2nd item
        pRenderListHead = pRenderListHead->pNext;

        // delete the texture of our previous head
        SDL_DestroyTexture(pToDelete->pTexture);

        // free our previous head from memory
        free(pToDelete);

        // debug output
        debugOutputComplete();
        return;
    }

    // create a temp renderObject pointer to increment the list
    renderObject *pCurrent = pRenderListHead;

    // while the next struct is not null and the next identifier is not our desired ID
    while (pCurrent->pNext != NULL && pCurrent->pNext->identifier != identifier) {
        // scoot over by one
        pCurrent = pCurrent->pNext;
    } // when this resolves, next will match desired ID or NULL

    // if we found the ID to remove
    if (pCurrent->pNext != NULL) {
        // copy our struct to delete to a temp var
        renderObject *pToDelete = pCurrent->pNext;

        // set our current next to what the deleted next pointed to
        pCurrent->pNext = pToDelete->pNext;

        // destroy the texture of our node to be deleted
        SDL_DestroyTexture(pToDelete->pTexture);

        // free our node from memory
        free(pToDelete);

        // debug output
        debugOutputComplete();
    }
    else{
        // if we couldnt find the ID, alarm
        printf("ERROR: COULD NOT FIND RENDER OBJECT WITH ID='%d' TO DELETE",identifier);
    }
}

// returns FIRST render object struct by ID, returns NULL if nonexistant
renderObject *getRenderObject(int identifier) {
    // traversal temp var
    renderObject *pCurrent = pRenderListHead;
    // while traversal var isnt null
    while (pCurrent != NULL) {
        // if our current ID matches the desired identifier
        if (pCurrent->identifier == identifier) {
            return pCurrent; // return our current
        }
        // else increment
        pCurrent = pCurrent->pNext;
    }
    // if no object exists with identifier, return NULL
    return NULL;
}

// load a font into memory and return a pointer to it
TTF_Font *loadFont(const char *pFontPath, int fontSize) {
    TTF_Font *pFont = TTF_OpenFont(pFontPath, fontSize);
    if (pFont == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return NULL;
    }
    return pFont;
}

// Create a texture from text string with specified font and color, returns NULL for failure
SDL_Texture *createTextTexture(const char *pText, TTF_Font *pFont, SDL_Color *pColor) {
    // create surface from parameters
    SDL_Surface *pSurface = TTF_RenderUTF8_Blended(pFont, pText, *pColor);
    
    // error out if surface creation failed
    if (pSurface == NULL) {
        printf("Failed to render text: %s\n", TTF_GetError());
        return NULL;
    }

    // create texture from surface
    SDL_Texture *pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);

    // error out if texture creation failed
    if (pTexture == NULL) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return NULL;
    }

    // free the surface memory
    SDL_FreeSurface(pSurface);

    // return the created texture
    return pTexture;
}

// Create a texture from image path, returns NULL for failure
SDL_Texture *createImageTexture(const char *pPath) {
    // create surface from loading the image
    SDL_Surface *pImage_surface = IMG_Load(pPath);
    
    // error out if surface load failed
    if (!pImage_surface) {
        printf("Error loading image: %s\n", IMG_GetError());
        return NULL;
    }

    // create texture from surface
    SDL_Texture *pTexture = SDL_CreateTextureFromSurface(pRenderer, pImage_surface);
    
    // error out if texture creation failed
    if (!pTexture) {
        printf("Error creating texture: %s\n", SDL_GetError());
        return NULL;
    }

    // release surface from memory
    SDL_FreeSurface(pImage_surface);
    
    // return the created texture
    return pTexture;
}

// add text to the render queue, returns the engine assigned ID of the object
int createText(int depth, float x,float y, float width, float height, char *pText, TTF_Font *pFont, SDL_Color *pColor, bool centered){
    addRenderObject(global_id,renderType_Text,depth,x,y,width,height,createTextTexture(pText,pFont,pColor),centered);
    global_id++; // increment the global ID for next object
    return global_id - 1; //return 1 less than after incrementation (id last item was assigned)
}

// add an image to the render queue, returns the engine assigned ID of the object
int createImage(int depth, float x, float y, float width, float height, char *pPath, bool centered){
    addRenderObject(global_id,renderType_Image,depth,x,y,width,height,createImageTexture(pPath),centered);
    global_id++;
    return global_id - 1;
}

/*
    method to create an engine button
    Takes in a string path to the background, font, text color, relative x, relative y, relative width, relative height
    CONSIDERATIONS / TODO: formatting the text such that it can be passed left, center, or right aligned and does not stretch to fill 
*/
// int createButton(int depth, float x, float y, float width, float height, char *text, TTF_Font *font, SDL_Color *color, bool centered, char *backgroundPath){
//     // create a render object of renderType_Button and save a pointer to the struct in a new list of Button Objects, return its assigned ID
//     // we are going to bake all textures into one so there is no fucking around with assosciating multiple render objects
//     // in theory this is also cheaper computationally (source: me assuming)
    
//     // actually, we have to handle the texture creation and ID assignment completely on our own, so theres that
    
    // TODO: THIS ALL GOES ON HOLD WHILE I REFACTOR addRenderObject

//     // renderObject obj = 
// }

// function that clears all non engine render objects (depth >= 0)
// TODO: refactor this and removeRenderObject() to send pointers to nodes to another function to genericise this
void clearAll(bool includeEngine) {
    // If our render list has zero items
    if (pRenderListHead == NULL) {
        printf("ERROR CLEARING ALL RENDER OBJECTS: HEAD IS NULL");
        return; // alarm and exit
    }

    // Initialize a previous node pointer to update pRenderListHead after deletions
    renderObject *pPrev = NULL;
    
    renderObject *pCurrent = pRenderListHead;
    while (pCurrent != NULL) {
        if (includeEngine || pCurrent->identifier >= 0) {
            // Delete the current object as we are either deleting everything or the current object is always deletable
            renderObject *pToDelete = pCurrent;
            printf("\033[0;31mRemove\033[0;37m render object [\033[0;33mid %d\033[0;37m]\t\t", pToDelete->identifier);
            pCurrent = pCurrent->pNext;
            SDL_DestroyTexture(pToDelete->pTexture);
            free(pToDelete);
            // If there was a previous node, update its next pointer
            if (pPrev != NULL) {
                pPrev->pNext = pCurrent;
            } else {
                // If there was no previous node, we deleted the head, so update pRenderListHead
                pRenderListHead = pCurrent;
            }
            debugOutputComplete();
        } else {
            // Pass as we have encountered an engine object that we don't want to delete
            pPrev = pCurrent;
            pCurrent = pCurrent->pNext;
        }
    }

    // If we cleared the whole list, set the pRenderListHead to NULL
    if (includeEngine) {
        pRenderListHead = NULL;
    }
}

// render everything in the scene
void renderAll() {
    int frameStart = SDL_GetTicks();

    // Set background color to black
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);

    // Clear the window with the set background color
    SDL_RenderClear(pRenderer);

    // create iteration var for render list
    renderObject *pCurrent = pRenderListHead;

    // while iteration var is not null
    while (pCurrent != NULL) {
        // check if current item is the fps counter
        if (pCurrent->type == renderType_Text && pCurrent->identifier == -1) {
            // allocate a new string
            char str[25];
                
            // Insert the fps number into the string
            sprintf(str, "fps: %d", fps);

            // increment the frame counter
            frameCounter++;

            // if we have waited long enough to update the display
            if (SDL_GetTicks() - fpsUpdateTime >= 250) {
                // get the elapsed time and scale it to our time factor to get fps
                fpsUpdateTime = SDL_GetTicks();
                fps = frameCounter * 4;
                frameCounter = 0; // reset counted frames
            }

            // Destroy old texture to prevent memory leak
            if (pCurrent->pTexture != NULL) {
                SDL_DestroyTexture(pCurrent->pTexture);
            }

            // Update texture with the new text TODO FIXME
            pCurrent->pTexture = createTextTexture(str, pEngineFont, pEngineFontColor);
        }

        // render our current object
        SDL_RenderCopy(pRenderer, pCurrent->pTexture, NULL, &(pCurrent->rect));
        
        // increment
        pCurrent = pCurrent->pNext;
    }

    // present our new changes to the renderer
    SDL_RenderPresent(pRenderer);

    // update the window to reflect the new renderer changes
    SDL_UpdateWindowSurface(pWindow);

    // if we arent on vsync we need to preform some frame calculations to delay next frame
    if(fpscap != -1){
        // set the end of the render frame
        int frameEnd = SDL_GetTicks();

        // calculate the current frame time
        int frameTime = frameEnd - frameStart;

        // check the desired FPS cap and add delay if needed
        if (frameTime < desiredFrameTime) {
            SDL_Delay(desiredFrameTime - frameTime);
        }
    }
}

// initialize graphics
void initGraphics(int screenWidth,int screenHeight, int windowMode, int framecap){
    // debug output
    printf("Attempting to initialize SDL... \t");

    // test for video init, alarm if failed
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        exit(1);
    }

    // debug: acknowledge SDL initialization
    debugOutputComplete(); 

    // test for window init, alarm if failed
    printf("Attempting to initialize window... \t");
    pWindow = SDL_CreateWindow("Stardust Crusaders Dating Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN | windowMode);
    if (pWindow == NULL) {
        printf("Window creation failed: %s\n", SDL_GetError()); // catch creation error
        exit(1);
    }
    // debug: acknowledge window initialization
    debugOutputComplete();

    // test for renderer init, alarm if failed
    printf("Attempting to initialize renderer...\n");
    
    // set our fps cap to the frame cap param
    // (-1) for vsync
    fpscap = framecap;
    desiredFrameTime = 1000 / fpscap;  

    // if vsync is on
    if(fpscap == -1) {
        printf("Starting renderer with vsync... \t");
        pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    }
    else {
        printf("Starting renderer with maxfps %d... \t",framecap);
        pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
    }

    if (pRenderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    // debug: acknowledge renderer initialization
    debugOutputComplete();

    // get our current aspect ratio
    float currentAspectRatio = (float)screenWidth / (float)screenHeight;

    // if aspect ratio is too wide
    if (currentAspectRatio >= targetAspectRatio) {
        // set our width to be the max that can accomidate the height
        virtualWidth = (int)(screenHeight * targetAspectRatio);
        virtualHeight = screenHeight;
        xOffset = (screenWidth - virtualWidth) / 2;
    } 
    // if aspect ratio is too tall
    else {
        // set our width to be the max that can fit
        virtualWidth = screenWidth;
        virtualHeight = screenWidth / targetAspectRatio;
        yOffset = (screenHeight - virtualHeight) / 2;
    }

    // debug outputs
    printf("Targeting aspect ratio: %f\n",targetAspectRatio);
    printf("Virtual Resolution: %dx%d\n",virtualWidth,virtualHeight);
    printf("(unused) X offset: %d\n(unused) Y offset: %d\n",xOffset,yOffset);

    // setup viewport with our virtual resolutions
    SDL_Rect viewport;
    viewport.x = xOffset;
    viewport.y = yOffset;
    viewport.w = virtualWidth;
    viewport.h = virtualHeight;
    SDL_RenderSetViewport(pRenderer, &viewport);
    
    // test for TTF init, alarm if failed
    printf("Attempting to initialize TTF... \t");
    if (TTF_Init() == -1) {
        printf("SDL2_ttf could not initialize! SDL2_ttf Error: %s\n", TTF_GetError());
        exit(1);
    }

    // debug: acknowledge TTF initialization
    debugOutputComplete();

    // test for IMG init, alarm if failed
    printf("Attempting to initialize IMG... \t");
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        SDL_Log("IMG_Init error: %s", IMG_GetError());
        exit(1);
    }

    // debug: acknowledge IMG initialization
    debugOutputComplete();

    // test for setting window icon, alarm if failed
    printf("Attempting to set window icon... \t");

    // load icon to surface
    SDL_Surface *pIconSurface = IMG_Load(getPath("images/icon.png"));
    if (pIconSurface == NULL) {
        SDL_Log("IMG_Load error: %s", IMG_GetError());
        exit(1);
    }
    // set icon
    SDL_SetWindowIcon(pWindow, pIconSurface);
    
    // release surface
    SDL_FreeSurface(pIconSurface);

    // debug: acknowledge window icon initialization
    debugOutputComplete();

    // set a start time for counting fps
    startTime = SDL_GetTicks();
}

// shuts down all initialzied graphics systems
void shutdownGraphics(){
    // shutdown TTF
    TTF_Quit();
    printf("\033[0;31mShut down TFT.\033[0;37m\n");

    // shutdown IMG
    IMG_Quit();
    printf("\033[0;31mShut down IMG.\033[0;37m\n");

    // shutdown renderer
    SDL_DestroyRenderer(pRenderer);
    printf("\033[0;31mRenderer destroyed.\033[0;37m\n");

    // shutdown window
    SDL_DestroyWindow(pWindow);
    printf("\033[0;31mWindow destroyed.\033[0;37m\n");
}