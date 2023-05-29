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
SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;
SDL_Renderer* renderer = NULL;
renderObject* renderListHead = NULL;

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

/*
    To avoid passing a pointer to an SDL color (which caused me great pain)
    it seems much easier to just have a dummy color to pass to the addRenderObject
    function when we are adding a object of type image which just needs a placeholder
    for the color and font in the struct
*/
SDL_Color dummyColor = {0, 0, 0, 0};

// constructor for render objects, invoked internally by renderText() and renderImage()
void addRenderObject(int identifier, renderObjectType type, int depth, int x, int y, int width, int height, SDL_Texture *texture, TTF_Font* font, SDL_Color color) {
    // debug output
    printf("\033[0;32mAdd\033[0;37m render object [\033[0;33mid %d\033[0;37m]\t\t",identifier);
    
    // create bounding rect from parameters
    SDL_Rect rect = {x, y, width, height};

    // construct and malloc new object
    renderObject *obj = (renderObject *)malloc(sizeof(renderObject));

    // self explanatory assignment of fields from parameters
    obj->identifier = identifier;
    obj->texture = texture;
    obj->rect = rect;
    obj->depth = depth;
    obj->next = NULL;
    obj->type = type;
    obj->font = font;
    obj->color = color;

    // if there are no renderObjects in the list, or the depth of this object is lower than the head
    if (renderListHead == NULL || obj->depth < renderListHead->depth) {
        // make this object the head
        obj->next = renderListHead; 
        renderListHead = obj;
    } 
    else {
        // iterate through the renderObject queue starting at the head
        renderObject* current = renderListHead;

        // while there is a next item and the next items depth is less than 
        // our current objects depth, keep going
        while (current->next != NULL && current->next->depth < obj->depth) {
            current = current->next;
        }

        // once we know where our object sits:
        obj->next = current->next; // make our object point to the one after our current
        current->next = obj; // make our current point to our object

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
}

// remove a render object from the queue by its identifier
void removeRenderObject(int identifier) {
    // debug output
    printf("\033[0;31mRemove\033[0;37m render object [\033[0;33mid %d\033[0;37m]\t\t",identifier);
    
    // if our render list has zero items
    if (renderListHead == NULL) {
        printf("ERROR REMOVING RENDER OBJECT: HEAD IS NULL");
        return;
        // alarm and pass
    }

    // if the head is the item we are looking to remove
    if (renderListHead->identifier == identifier) {
        // pop our head into a new temp object
        renderObject* toDelete = renderListHead;

        // set our head to the previous 2nd item
        renderListHead = renderListHead->next;

        // delete the texture of our previous head
        SDL_DestroyTexture(toDelete->texture);

        // free our previous head from memory
        free(toDelete);

        // debug output
        debugOutputComplete();
        return;
    }

    // create a temp renderObject pointer to increment the list
    renderObject* current = renderListHead;

    // while the next struct is not null and the next identifier is not our desired ID
    while (current->next != NULL && current->next->identifier != identifier) {
        // scoot over by one
        current = current->next;
    } // when this resolves, next will match desired ID or NULL

    // if we found the ID to remove
    if (current->next != NULL) {
        // copy our struct to delete to a temp var
        renderObject* toDelete = current->next;

        // set our current next to what the deleted next pointed to
        current->next = toDelete->next;

        // destroy the texture of our node to be deleted
        SDL_DestroyTexture(toDelete->texture);

        // free our node from memory
        free(toDelete);

        // debug output
        debugOutputComplete();
    }
    else{
        // if we couldnt find the ID, alarm
        printf("ERROR: COULD NOT FIND RENDER OBJECT WITH ID='%d' TO DELETE",identifier);
    }
}

// returns FIRST render object struct by ID, returns NULL if nonexistant
renderObject* getRenderObject(int identifier) {
    // traversal temp var
    renderObject* current = renderListHead;
    // while traversal var isnt null
    while (current != NULL) {
        // if our current ID matches the desired identifier
        if (current->identifier == identifier) {
            return current; // return our current
        }
        // else increment
        current = current->next;
    }
    // if no object exists with identifier, return NULL
    return NULL;
}

// load a font into memory and return a pointer to it
TTF_Font* loadFont(const char* fontPath, int fontSize) {
    TTF_Font* font = TTF_OpenFont(fontPath, fontSize);
    if (font == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return NULL;
    }
    return font;
}

// Create a texture from text string with specified font and color, returns NULL for failure
SDL_Texture* createTextTexture(const char* text, TTF_Font* font, SDL_Color color) {
    // create surface from parameters
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    
    // error out if surface creation failed
    if (surface == NULL) {
        printf("Failed to render text: %s\n", TTF_GetError());
        return NULL;
    }

    // create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    // error out if texture creation failed
    if (texture == NULL) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return NULL;
    }

    // free the surface memory
    SDL_FreeSurface(surface);

    // return the created texture
    return texture;
}

// Create a texture from image path, returns NULL for failure
SDL_Texture* createImageTexture(const char* path) {
    // create surface from loading the image
    SDL_Surface* image_surface = IMG_Load(path);
    
    // error out if surface load failed
    if (!image_surface) {
        printf("Error loading image: %s\n", IMG_GetError());
        return NULL;
    }

    // create texture from surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image_surface);
    
    // error out if texture creation failed
    if (!texture) {
        printf("Error creating texture: %s\n", SDL_GetError());
        return NULL;
    }

    // release surface from memory
    SDL_FreeSurface(image_surface);
    
    // return the created texture
    return texture;
}

// add text to the render queue, returns the engine assigned ID of the object
int renderText(int depth, int x,int y, int width, int height, char *text, TTF_Font *font, SDL_Color color){
    addRenderObject(global_id,renderType_Text,depth,x,y,width,height,createTextTexture(text,font,color),font,color);
    global_id++; // increment the global ID for next object
    return global_id - 1; //return 1 less than after incrementation (id last item was assigned)
}

// add an image to the render queue, returns the engine assigned ID of the object
int renderImage(int depth, int x, int y, int width, int height, char *path){
    addRenderObject(global_id,renderType_Image,depth,x,y,width,height,createImageTexture(path),NULL,dummyColor);
    global_id++;
    return global_id - 1;
}

// render everything in the scene
void renderAll() {
    int frameStart = SDL_GetTicks();

    // Set background color to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Clear the window with the set background color
    SDL_RenderClear(renderer);

    // create iteration var for render list
    renderObject* current = renderListHead;

    // while iteration var is not null
    while (current != NULL) {
        // check if current item is the fps counter
        if (current->type == renderType_Text && current->identifier == -1) {
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
            if (current->texture != NULL) {
                SDL_DestroyTexture(current->texture);
            }

            // Update texture with the new text
            current->texture = createTextTexture(str, current->font, current->color);
        }

        // render our current object
        SDL_RenderCopy(renderer, current->texture, NULL, &(current->rect));
        
        // increment
        current = current->next;
    }

    // present our new changes to the renderer
    SDL_RenderPresent(renderer);

    // update the window to reflect the new renderer changes
    SDL_UpdateWindowSurface(window);

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
    window = SDL_CreateWindow("Stardust Crusaders Dating Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN | windowMode);
    if (window == NULL) {
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
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    }
    else {
        printf("Starting renderer with maxfps %d... \t",framecap);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }

    if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        exit(1);
    }

    // debug: acknowledge renderer initialization
    debugOutputComplete();
    
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
    SDL_Surface *iconSurface = IMG_Load("resources/images/icon.png");
    if (iconSurface == NULL) {
        SDL_Log("IMG_Load error: %s", IMG_GetError());
        exit(1);
    }
    // set icon
    SDL_SetWindowIcon(window, iconSurface);
    
    // release surface
    SDL_FreeSurface(iconSurface);

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
    SDL_DestroyRenderer(renderer);
    printf("\033[0;31mRenderer destroyed.\033[0;37m\n");

    // shutdown window
    SDL_DestroyWindow(window);
    printf("\033[0;31mWindow destroyed.\033[0;37m\n");
}