/*
    TODO: GRAPHICS
    - some sort of system for re render only updated renderObjects
*/

#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <jansson.h>

#include "engine.h"
#include "graphics.h"
#include "audio.h"
#include "logging.h"
#include "variant.h"

// TODO FIXME BAD BAD BAD BAD
#include "../data.h"

// define globals for file
SDL_Window *pWindow = NULL;
SDL_Surface *pScreenSurface = NULL;
SDL_Renderer *pRenderer = NULL;

renderObject *pRenderListHead = NULL;
button *pButtonListHead = NULL;

// int that increments each renderObject created, allowing new unique id's to be assigned
int global_id = 0;
int objectCount = 0;
int lastObjectCount = 0;
int lastChunkCount = 0;
int lastLinesWritten = 0;

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

bool forceRefresh = false;

int currentResolutionWidth = 1920;
int currentResolutionHeight = 1080;

// create a cache to hold textures colors and fonts
VariantCollection* cache;

// helper function to get renderObjectType as a string from the enum name
char *getRenderObjectTypeString(renderObjectType type) {
    switch (type) {
        case renderType_Text:
            return "Text";
        case renderType_Image:
            return "Image";
        case renderType_Button:
            return "Button";
        default:
            return "Unknown";
    }
}

// constructor for render objects, invoked internally by createText() and createImage()
// NOTE: this function inserts highest depth objects at the front of the list
void addRenderObject(int identifier, renderObjectType type, int depth, float x, float y, float width, float height, SDL_Texture *pTexture, bool centered, bool cachedTexture) {
    // translate our relative floats into actual screen coordinates for rendering
    // TODO: consider genericizing this into a function
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
    pObj->cachedTexture = cachedTexture;

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
    
    if(centered){
        // char buffer[100];
        // snprintf(buffer, sizeof(buffer),  "Added renderObject %s id#%d centered at (%d,%d) %dx%d\n",getRenderObjectTypeString(type),identifier,objX,objY,objWidth,objHeight);
        // logMessage(debug, buffer);
    }
    else{
        // char buffer[100];
        // snprintf(buffer, sizeof(buffer),  "Added renderObject %s id#%d absolutely at (%d,%d) %dx%d\n",getRenderObjectTypeString(type),identifier,objX,objY,objWidth,objHeight);
        // logMessage(debug, buffer);
    }
    objectCount++;
}

// remove a render object from the queue by its identifier
void removeRenderObject(int identifier) {
    // debug output
    // char buffer[100];
    // snprintf(buffer, sizeof(buffer),  "Remove render object id#%d\n",identifier);
    // logMessage(debug, buffer);
    
    // if our render list has zero items
    if (pRenderListHead == NULL) {
        logMessage(warning, "ERROR REMOVING RENDER OBJECT: HEAD IS NULL\n");
        return;
        // alarm and pass
    }

    // if the head is the item we are looking to remove
    if (pRenderListHead->identifier == identifier) {
        // pop our head into a new temp object
        renderObject *pToDelete = pRenderListHead;

        // set our head to the previous 2nd item
        pRenderListHead = pRenderListHead->pNext;

        // delete the texture of our previous head (if its not cached)
        if(!pToDelete->cachedTexture){
            SDL_DestroyTexture(pToDelete->pTexture);
        } // this allows any cache textures to be freed separately

        // free our previous head from memory
        free(pToDelete);

        // finalize
        objectCount--;
        return;
    }

    // create a temp renderObject pointer to increment the list
    renderObject *pCurrent = pRenderListHead;

    /*
        [1,x]->[2,x]->[3,0x0]
    HEAD ^
    */

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

        // destroy the texture of our node to be deleted (if its not cached)
        if(!pToDelete->cachedTexture){
            SDL_DestroyTexture(pToDelete->pTexture);
        } // this allows any cache textures to be freed separately

        // free our node from memory
        free(pToDelete);

        objectCount--;
    }
    else{
        // if we couldnt find the ID, alarm
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "ERROR: COULD NOT FIND RENDER OBJECT WITH ID='%d' TO DELETE\n",identifier);
        logMessage(error, buffer);
    }
}

void removeButton(int id){
    // step through button LL and find which node has the ID of the renderobject passed
    // then, we can remove that node from the LL and free it and call removeRenderObject() on the renderObject
    if(pButtonListHead == NULL){
        // logMessage(warning, "ERROR REMOVING BUTTON: HEAD IS NULL\n");
    }
    else{
        button *pCurrent = pButtonListHead;
        while(pCurrent->pNext != NULL && pCurrent->pNext->pObject->identifier != id){
            pCurrent = pCurrent->pNext;
        }
        if(pCurrent->pNext != NULL){
            button *pToDelete = pCurrent->pNext;
            pCurrent->pNext = pToDelete->pNext;

            // including jansson pepega code
            json_decref(pToDelete->callbackData->pJson);
            free(pToDelete->callbackData->callbackType);
            free(pToDelete->callbackData);

            free(pToDelete);
            removeRenderObject(id);
        }
        else{
            char buffer[100];
            snprintf(buffer, sizeof(buffer),  "ERROR: COULD NOT FIND BUTTON WITH ID#%d TO DELETE\n",id);
            logMessage(error, buffer);
        }
    }
}

// helper function for clearAll() to remove all buttons from the button LL and their render objects
void clearAllButtons(){
    if(pButtonListHead == NULL){
        // logMessage(warning, "ERROR REMOVING ALL BUTTONS: HEAD IS NULL\n");
    }
    else{
        button *pCurrent = pButtonListHead;
        while(pCurrent != NULL){
            button *pToDelete = pCurrent;
            pCurrent = pCurrent->pNext;

            // char buffer[100];
            // snprintf(buffer, sizeof(buffer),  "Remove button object id#%d\n", pToDelete->pObject->identifier);
            // logMessage(debug, buffer);
            removeRenderObject(pToDelete->pObject->identifier);
            
            // we decref the json_t object inside the callbackData struct, because its a new ref so it will be freed
            json_decref(pToDelete->callbackData->pJson);

            // we free our dynamically allocated fields
            free(pToDelete->callbackData->callbackType); // free the malloced type string
            free(pToDelete->callbackData); // free the malloced callbackData struct
            
            free(pToDelete); // free button object
        }
        pButtonListHead = NULL;
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
// TODO: evaluate where this stands in relation to getFont functionality, does this just extend the backend of get Font?
TTF_Font *loadFont(const char *pFontPath, int fontSize) {
    if(fontSize > 500){
        logMessage(error, "ERROR: FONT SIZE TOO LARGE\n");
        return NULL;
    }
    char *fontpath = getPathStatic(pFontPath);
    if(access(fontpath, F_OK) == -1){
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Could not access file '%s'.\n", fontpath);
        logMessage(error, buffer);
    }
    TTF_Font *pFont = TTF_OpenFont(fontpath, fontSize);
    if (pFont == NULL) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Failed to load font: %s\n", TTF_GetError());
        logMessage(error, buffer);
        return NULL;
    }
    char buffer[100];
    snprintf(buffer, sizeof(buffer),  "Loaded font: %s\n", pFontPath);
    logMessage(debug, buffer);
    return pFont;
}

// Create a texture from text string with specified font and color, returns NULL for failure
SDL_Texture *createTextTexture(const char *pText, TTF_Font *pFont, SDL_Color *pColor) {
    // create surface from parameters
    SDL_Surface *pSurface = TTF_RenderUTF8_Blended(pFont, pText, *pColor); // MEMLEAK: valgrind says so but its not my fault, internal in TTF
    
    // error out if surface creation failed
    if (pSurface == NULL) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Failed to render text: %s\n", TTF_GetError());
        logMessage(error, buffer);
        return NULL;
    }

    // create texture from surface
    SDL_Texture *pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);

    // error out if texture creation failed
    if (pTexture == NULL) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Failed to create texture: %s\n", SDL_GetError());
        logMessage(error, buffer);
        return NULL;
    }

    // free the surface memory
    SDL_FreeSurface(pSurface);

    // return the created texture
    return pTexture;
}

// Create a texture from image path, returns its texture pointer and a flag on whether
// or not the texture is also cached
struct textureInfo createImageTexture(char *pPath, bool shouldCache) {

    // try to get it from cache TODO: should we wrap this in if shouldCache?
    Variant *pVariant = getVariant(cache, pPath);

    if (pVariant != NULL) { // found in cache
        // logMessage(debug, "Found texture in cache\n");

        struct textureInfo ret = {pVariant->textureValue, true};

        return ret;
    }
    else{ // not found in cache
        if(access(getPathStatic(pPath), F_OK) == -1){
            char buffer[100];
            snprintf(buffer, sizeof(buffer),  "Could not access file '%s'.\n", pPath);
            logMessage(error, buffer);
        }

        // create surface from loading the image
        SDL_Surface *pImage_surface = IMG_Load(getPathStatic(pPath));
        
        // error out if surface load failed
        if (!pImage_surface) {
            char buffer[100];
            snprintf(buffer, sizeof(buffer),  "Error loading image: %s\n", IMG_GetError());
            logMessage(error, buffer);
            exit(1); // FIXME
        }

        // create texture from surface
        SDL_Texture *pTexture = SDL_CreateTextureFromSurface(pRenderer, pImage_surface);
        
        // error out if texture creation failed
        if (!pTexture) {
            char buffer[100];
            snprintf(buffer, sizeof(buffer),  "Error creating texture: %s\n", SDL_GetError());
            logMessage(error, buffer);
            exit(1); // FIXME
        }

        // release surface from memory
        SDL_FreeSurface(pImage_surface);
        if(shouldCache){ // we are going to cache it
            // cache the texture
            Variant variant;
            variant.type = VARIANT_TEXTURE;
            variant.textureValue = pTexture;

            addVariant(cache, pPath, variant);

            char buffer[100];
            snprintf(buffer, sizeof(buffer),  "Cached a texture. key: %s\n", pPath);
            logMessage(debug, buffer);

            if(getVariant(cache, pPath) == NULL){
                logMessage(error, "ERROR CACHING TEXTURE\n");
            }

            struct textureInfo ret = {pTexture, true};
            
            // return the created texture
            return ret;

        }
        else{ // we are not going to cache it
            struct textureInfo ret = {pTexture, false}; 

            return ret;
        }
    }
}

/*
    takes in a key (relative resource path to a font) and returns a 
    pointer to the loaded font (caches the font if it doesnt exist in cache)
*/
TTF_Font *getFont(char *key){
    Variant *v = getVariant(cache, key);
    if(v == NULL){
        Variant fontVariant;
        fontVariant.type = VARIANT_FONT;
        fontVariant.fontValue = loadFont(key,100);

        addVariant(cache, key, fontVariant);
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Cached a font. key: %s\n", key);
        logMessage(debug, buffer);
        v = getVariant(cache, key);
    }
    else{
        // logMessage(debug, "Found cached font.\n");
    }
    return v->fontValue;
}

/*
    takes in a key (relative resource path to a font) and returns a 
    pointer to the loaded font (caches the font if it doesnt exist in cache)

    This is mainly here for the game, the engine cant utilize this super well as
    it doesnt have an understanding of named colors, it can just cache them for quick
    access by the game
*/
SDL_Color *getColor(char *key, SDL_Color color){
    Variant *v = getVariant(cache, key);
    if(v == NULL){
        Variant colorVariant;
        colorVariant.type = VARIANT_COLOR;
        colorVariant.colorValue = color;

        addVariant(cache, key, colorVariant);
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Cached a color. key: %s\n", key);
        logMessage(debug, buffer);
        v = getVariant(cache, key);
    }
    else{
        // logMessage(debug, "Found cached color.\n");
    }
    SDL_Color *pColor = &v->colorValue;
    return pColor;
}

// add text to the render queue, returns the engine assigned ID of the object
int createText(int depth, float x,float y, float width, float height, char *pText, TTF_Font *pFont, SDL_Color *pColor, bool centered){
    addRenderObject(global_id,renderType_Text,depth,x,y,width,height,createTextTexture(pText,pFont,pColor),centered,false); // FIXME: temp false flag here text is never cached, pass in struct?
    global_id++; // increment the global ID for next object
    return global_id - 1; //return 1 less than after incrementation (id last item was assigned)
}

// add an image to the render queue, returns the engine assigned ID of the object
int createImage(int depth, float x, float y, float width, float height, char *pPath, bool centered){
    struct textureInfo info = createImageTexture(pPath,true);
    addRenderObject(global_id,renderType_Image,depth,x,y,width,height,info.pTexture,centered,info.cached);
    global_id++;
    return global_id - 1;
}

/*
    method to create an engine button
    Takes in a string path to the background, font, text color, relative x, relative y, relative width, relative height
    CONSIDERATIONS / TODO: 
    - formatting the text such that it can be passed left, center, or right aligned and does not stretch to fill 
    - refactor texture rendering to external function so button textures can be generated and replaced externally in the future, for now buttons are static (maybe that texture can be auto modified by pointer in struct)
*/
int createButton(int depth, float x, float y, float width, float height, char *pText, TTF_Font *pFont, SDL_Color *pColor, bool centered, char *pBackgroundPath, struct callbackData *data) {
    // translate our relative floats into actual screen coordinates for rendering TODO: consider genericizing this into a function
    // int realX = (int)(x * (float)virtualWidth); // + xOffset;
    // int realY = (int)(y * (float)virtualHeight); // + yOffset;
    int realWidth = (int)(width * (float)virtualWidth);
    int realHeight = (int)(height * (float)virtualHeight);

    SDL_Texture *textTexture = createTextTexture(pText, pFont, pColor);

    if (textTexture == NULL) {
        logMessage(error, "ERROR CREATING TEXT TEXTURE FOR BUTTON\n");
        return intFail;
    }

    struct textureInfo info = createImageTexture(pBackgroundPath,true);
    SDL_Texture *pImageTexture = info.pTexture;

    if(pImageTexture == NULL){
        logMessage(error, "ERROR CREATING IMAGE TEXTURE FOR BUTTON\n");
        SDL_DestroyTexture(textTexture);
        return intFail;
    }

    SDL_Texture* buttonTexture = SDL_CreateTexture(pRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, realWidth, realHeight);

    if (buttonTexture == NULL) {
        logMessage(error, "ERROR CREATING BUTTON TEXTURE\n");
        SDL_DestroyTexture(textTexture);
        return intFail;
    }

    // Set the new texture as the render target
    SDL_SetRenderTarget(pRenderer, buttonTexture);

    // Render the background image onto the new texture
    SDL_Rect backgroundRect = {0, 0, realWidth, realHeight};
    SDL_RenderCopy(pRenderer, pImageTexture, NULL, &backgroundRect);

    // Get dimensions of the text texture
    int textWidth, textHeight;
    SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);

    float widthRatio = (float)realWidth / textWidth;
    float heightRatio = (float)realHeight / textHeight;
    float scale = fminf(widthRatio, heightRatio);

    int scaledTextWidth = (int)(textWidth * scale);
    int scaledTextHeight = (int)(textHeight * scale);

    // Calculate the position of the text to center it within the button
    SDL_Rect textRect;
    textRect.x = (realWidth - scaledTextWidth) / 2;
    textRect.y = (realHeight - scaledTextHeight) / 2;
    textRect.w = scaledTextWidth;
    textRect.h = scaledTextHeight;

    // Render the text onto the new texture
    SDL_RenderCopy(pRenderer, textTexture, NULL, &textRect);

    // Reset the render target to the default
    SDL_SetRenderTarget(pRenderer, NULL);

    global_id++; // to stay consistant, increment now and refer to global_id - 1 when accessing ID

    addRenderObject(global_id - 1,renderType_Button,depth,x,y,width,height,buttonTexture,centered,false); // NOTE: HARD CODED NOCACHE

    renderObject *pObj = getRenderObject(global_id - 1);

    // construct and malloc new button
    button *pButton = (button *)malloc(sizeof(button));
    pButton->pObject = pObj;
    pButton->pNext = NULL;
    pButton->callbackData = data;

    // Add the new button to the linked list
    // (sorted by depth, highest at head)
    if(pButtonListHead == NULL){
        pButtonListHead = pButton;
    }
    else{
        button *pCurrent = pButtonListHead;
        while(pCurrent->pNext != NULL && pCurrent->pNext->pObject->depth < pButton->pObject->depth){
            pCurrent = pCurrent->pNext;
        }
        pButton->pNext = pCurrent->pNext;
        pCurrent->pNext = pButton;
    }

    // Cleanup
    SDL_DestroyTexture(textTexture);

    return global_id - 1; // for consistancy
}

// function that clears all non engine render objects (depth >= 0)
// TODO: refactor this and removeRenderObject() to send pointers to nodes to another function to genericise this
void clearAll(bool includeEngine) {


    // If our render list has zero items
    if (pRenderListHead == NULL) {
        // logMessage(warning, "ERROR CLEARING ALL RENDER OBJECTS: HEAD IS NULL\n");
        return; // alarm and exit
    }

    // attempt to clear all buttons
    clearAllButtons();

    renderObject *pCurrent = pRenderListHead;
    renderObject *pNext = NULL;
    renderObject *pPrev = NULL; // Declaration of pPrev

    while (pCurrent != NULL) {
        pNext = pCurrent->pNext;

        if (includeEngine || pCurrent->identifier >= 0) {
            // Delete the current object as we are either deleting everything or the current object is always deletable
            // char buffer[100];
            // snprintf(buffer, sizeof(buffer),  "Remove render object id#%d\n", pCurrent->identifier);
            // logMessage(debug, buffer);

            // Delete the texture of our current object (if its not cached)
            if(!pCurrent->cachedTexture){
                SDL_DestroyTexture(pCurrent->pTexture);
            } // this allows any cache textures to be freed separately
            
            free(pCurrent);
            objectCount--;
        } else {
            // Pass as we have encountered an engine object that we don't want to delete
            pPrev = pCurrent;
        }

        pCurrent = pNext;

        // Update pPrev if necessary
        if (pPrev != NULL) {
            pPrev->pNext = pCurrent;
        } else {
            pRenderListHead = pCurrent;
        }
    }

    // If we cleared the whole list, set pRenderListHead to NULL
    if (includeEngine && pPrev == NULL) {
        pRenderListHead = NULL;
    }
    // clear cache
    clearVariantCollection(cache); // TODO: evaluate when cache should be cleared intelligently with refcounts
}

// function to allow externel signal to force display refresh for debug overlay
void debugForceRefresh(){
    forceRefresh = true;
}

//TODO: OPTIMIZATION -> switch if clauses because currently ID always runs but the inside case isnt always true
// render everything in the scene
void renderAll() {
    int frameStart = SDL_GetTicks();

    // Set background color to black
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);

    // Clear the window with the set background color
    SDL_RenderClear(pRenderer);

    // create iteration var for render list
    renderObject *pCurrent = pRenderListHead;

    // Get paint start timestamp
    Uint32 paintStartTime = SDL_GetTicks();

    // while iteration var is not null
    while (pCurrent != NULL) {
        // check if current item is the fps counter
        if (pCurrent->identifier == -1) {
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
            pCurrent->pTexture = createTextTexture(str, pEngineFont2, pEngineFontColor);
        }

        // check if current item is the render object counter
        if (pCurrent->identifier == -2) {
            // Check if the object count has changed
            if (objectCount != lastObjectCount || forceRefresh) {
                // Update the previous object count value
                lastObjectCount = objectCount;

                // allocate a new string
                char strObjCount[25];

                // Insert the render object count number into the string
                sprintf(strObjCount, "render objects: %d", objectCount);

                // Destroy old texture to prevent memory leak
                if (pCurrent->pTexture != NULL) {
                    SDL_DestroyTexture(pCurrent->pTexture);
                }

                // Update texture with the new text TODO FIXME
                pCurrent->pTexture = createTextTexture(strObjCount, pEngineFont2, pEngineFontColor);
            }
        }

        // check if current item is the audio chunk counter
        if (pCurrent->identifier == -3) {
            // Check if the object count has changed
            if (totalChunks != lastChunkCount || forceRefresh) {
                // Update the previous object count value
                lastChunkCount = totalChunks;

                // allocate a new string
                char strChkCount[25];

                // Insert the render object count number into the string
                sprintf(strChkCount, "audio chunks: %d", totalChunks);

                // Destroy old texture to prevent memory leak
                if (pCurrent->pTexture != NULL) {
                    SDL_DestroyTexture(pCurrent->pTexture);
                }

                // Update texture with the new text TODO FIXME
                pCurrent->pTexture = createTextTexture(strChkCount, pEngineFont2, pEngineFontColor);
            }
        }

        // check if current item is the log lines
        // TODO: in the future this is where a dev console with log output could be
        if (pCurrent->identifier == -4) {
            // Check if the object count has changed
            if (linesWritten != lastLinesWritten || forceRefresh) {
                // Update the previous object count value
                lastLinesWritten = linesWritten;

                // allocate a new string
                char strLinCount[25];

                // Insert the render object count number into the string
                sprintf(strLinCount, "log lines: %d", linesWritten);

                // Destroy old texture to prevent memory leak
                if (pCurrent->pTexture != NULL) {
                    SDL_DestroyTexture(pCurrent->pTexture);
                }

                // Update texture with the new text TODO FIXME
                pCurrent->pTexture = createTextTexture(strLinCount, pEngineFont2, pEngineFontColor);
            }
        }

        // render our current object
        SDL_RenderCopy(pRenderer, pCurrent->pTexture, NULL, &(pCurrent->rect));
        
        // increment
        pCurrent = pCurrent->pNext;
    }

    // Get paint end timestamp
    Uint32 paintEndTime = SDL_GetTicks();

    // Calculate paint time
    Uint32 paintTime = paintEndTime - paintStartTime;

    if (SDL_GetTicks() - fpsUpdateTime >= 250) {
        // Update ID -5 texture with paint time
        pCurrent = pRenderListHead;
        while (pCurrent != NULL) {
            if (pCurrent->identifier == -5) {
                char paintTimeString[30];

                sprintf(paintTimeString, "paint time: %d ms", paintTime);

                if (pCurrent->pTexture != NULL) {
                    SDL_DestroyTexture(pCurrent->pTexture);
                }

                pCurrent->pTexture = createTextTexture(paintTimeString, pEngineFont2, pEngineFontColor);

                // Render the updated paint time texture
                SDL_RenderCopy(pRenderer, pCurrent->pTexture, NULL, &(pCurrent->rect));

                break;
            }

            pCurrent = pCurrent->pNext;
        }
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

    if(forceRefresh){
        forceRefresh = false;
    }

}

// function that traverses our LL of buttons and returns the highest depth
// button clicked by ID, NULL if none
void checkClicked(int x, int y){
    // create a temp button pointer to increment the list
    button *pCurrent = pButtonListHead;

    // while the next struct is not null
    while (pCurrent != NULL) {
        // check if we have clicked inside the button
        if (x >= pCurrent->pObject->rect.x +xOffset &&
            x <= pCurrent->pObject->rect.x + pCurrent->pObject->rect.w +xOffset &&
            y >= pCurrent->pObject->rect.y +yOffset &&
            y <= pCurrent->pObject->rect.y + pCurrent->pObject->rect.h + yOffset) 
        {
            // run the buttons callback if its not null
            if(pCurrent->callbackData->callback != NULL && pCurrent->callbackData != NULL && pCurrent->callbackData->pJson != NULL){
                logMessage(debug, "Button clicked, running callback\n");
                pCurrent->callbackData->callback(pCurrent->callbackData);
                logMessage(debug, "Callback finished\n");
                return;
            }
            else{
                logMessage(warning, "ERROR: CLICKED BUTTON CALLBACK IS NULL\n");
                return;
            }
            // return pCurrent->pObject->identifier; // return our current
        }
        // else increment
        pCurrent = pCurrent->pNext;
    }
    // if no object exists with identifier, return NULL
    //return intFail;
}

// method to ensure our game content is centered and scaled well
void setViewport(int screenWidth, int screenHeight){
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
    char buffer[100];
    snprintf(buffer, sizeof(buffer),  "Targeting aspect ratio: %f\n",targetAspectRatio);
    logMessage(debug, buffer);
    snprintf(buffer, sizeof(buffer),  "Virtual Resolution: %dx%d\n",virtualWidth,virtualHeight);
    logMessage(debug, buffer);
    snprintf(buffer, sizeof(buffer),  "(unused) offset: %dx%d\n",xOffset,yOffset);
    logMessage(debug, buffer);

    // setup viewport with our virtual resolutions
    SDL_Rect viewport;
    viewport.x = xOffset;
    viewport.y = yOffset;
    viewport.w = virtualWidth;
    viewport.h = virtualHeight;
    SDL_RenderSetViewport(pRenderer, &viewport);
}

// /*
//     method to change the window mode, flag passed in will set the window mode
//     method will also change the resolution to match the new window mode:
//     fullscreen - will auto detect screen resolution and set it
//     windowed - will set the resolution to 1920x1080
// */
void changeWindowMode(Uint32 flag)
{
    int success = SDL_SetWindowFullscreen(pWindow, flag);
    if(success < 0) 
    {
        logMessage(error, "ERROR: COULD NOT CHANGE WINDOW MODE\n");
        return;
    }
    else
    {
        logMessage(info, "Changed window mode.\n");
    }

    if(flag == 0){
        changeResolution(1920, 1080);
    }
    else{
        SDL_DisplayMode displayMode;
        if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
            logMessage(error, "SDL_GetCurrentDisplayMode failed!\n");
            return;
        }
        int screenWidth = displayMode.w;
        int screenHeight = displayMode.h;
        
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Inferred screen size: %dx%d\n", screenWidth, screenHeight);
        logMessage(debug, buffer);

        changeResolution(screenWidth, screenHeight);
    }
}

// /*
//     Shuts down current renderer, creates a new renderer with or withou
//     vsync according to passed bool
// */
void setVsync(bool enabled) {
    SDL_DestroyRenderer(pRenderer);
    logMessage(debug, "Renderer destroyed to toggle vsync.\n");

    uint32_t flags = SDL_RENDERER_ACCELERATED;
    if (enabled) {
        flags |= SDL_RENDERER_PRESENTVSYNC;
    }

    pRenderer = SDL_CreateRenderer(pWindow, -1, flags);
    logMessage(debug, "Renderer re-created.\n");

    if (pRenderer == NULL) {
        logMessage(warning,"ERROR RE-CREATING RENDERER\n");     
        exit(1);
    }

    // NOTE: in fullscreen, resetting the render means we need to reset our viewport
    setViewport(currentResolutionWidth,currentResolutionHeight);
}

// /*
//     Changes the game fps cap to the passed integer
//     TODO: add checks for if we need to change vsync to save performance
// */
void changeFPS(int cap){
    toggleOverlay(); // NOTE: overlay bugs out when we change renderer so we have to toggle it twice
    if(cap == -1){
        setVsync(true);
    }
    else{
        setVsync(false);
        fpscap = cap;
        desiredFrameTime = 1000 / fpscap;
    }
    toggleOverlay();
}

/*
    returns a ScreenSize struct of the current resolution w,h
*/
struct ScreenSize getCurrentResolution(){
    struct ScreenSize screensize = {currentResolutionWidth,currentResolutionHeight}; // TODO: this sucks
    return screensize;
}

/*
    Changes the game resolution to the passed width and height
*/
void changeResolution(int width, int height) {
    SDL_SetWindowSize(pWindow, width, height);
    setViewport(width, height);
    currentResolutionWidth = width;
    currentResolutionHeight = height;
}

// initialize graphics
void initGraphics(int screenWidth,int screenHeight, int windowMode, int framecap){
    // test for video init, alarm if failed
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "SDL initialization failed: %s\n", SDL_GetError());
        logMessage(debug, buffer);
        exit(1);
    }

    logMessage(info, "SDL initialized.\n");

    // test for window init, alarm if failed
    pWindow = SDL_CreateWindow("Stardust Crusaders Dating Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN | windowMode);
    if (pWindow == NULL) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Window creation failed: %s\n", SDL_GetError());
        logMessage(debug, buffer);
        exit(1);
    }
    
    logMessage(info, "Window initialized.\n");
    
    // set our fps cap to the frame cap param
    // (-1) for vsync
    fpscap = framecap;
    desiredFrameTime = 1000 / fpscap;  

    // if vsync is on
    if(fpscap == -1) {
        logMessage(info, "Starting renderer with vsync... \n");
        pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    }
    else {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Starting renderer with maxfps %d... \n",framecap);
        logMessage(debug, buffer);
        pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
    }

    if (pRenderer == NULL) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        logMessage(error, buffer);
        exit(1);
    }

    // set our viewport to the screen size with neccessary computed offsets
    setViewport(screenWidth, screenHeight);
    
    // test for TTF init, alarm if failed
    if (TTF_Init() == -1) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "SDL2_ttf could not initialize! SDL2_ttf Error: %s\n", TTF_GetError());
        logMessage(error, buffer);
        exit(1);
    }
    logMessage(info, "TTF initialized.\n");

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "IMG_Init error: %s", IMG_GetError());
        logMessage(error, buffer);
        exit(1);
    }
    logMessage(info, "IMG initialized.\n");

    // initialize cache
    cache = createVariantCollection();

    // load icon to surface
    SDL_Surface *pIconSurface = IMG_Load(getPathStatic("images/icon.png"));
    if (pIconSurface == NULL) {
        char buffer[100];
        snprintf(buffer, sizeof(buffer),  "IMG_Load error: %s", IMG_GetError());
        logMessage(error, buffer);
        exit(1);
    }
    // set icon
    SDL_SetWindowIcon(pWindow, pIconSurface);
    
    // release surface
    SDL_FreeSurface(pIconSurface);

    logMessage(info, "Window icon set.\n");

    // set a start time for counting fps
    startTime = SDL_GetTicks();
}

// shuts down all initialzied graphics systems
void shutdownGraphics(){
    // remove all render objects and free everything (including cache)
    clearAll(true);
    
    // destroy cache
    destroyVariantCollection(cache);

    // shutdown TTF
    TTF_Quit();
    logMessage(info, "Shut down TTF.\n");

    // shutdown IMG
    IMG_Quit();
    logMessage(info, "Shut down IMG.\n");

    // shutdown renderer
    SDL_DestroyRenderer(pRenderer);
    logMessage(info, "Shut down renderer.\n");

    // shutdown window
    SDL_DestroyWindow(pWindow);
    logMessage(info, "Shut down window.\n");
}