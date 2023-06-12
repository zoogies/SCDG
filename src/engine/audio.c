/*
    AUDIO TODO:
    - volume control
    - allow channel to be specified or auto assigned
    - returning out of this file what channel a sound was assigned to so it can be interrupted
      or maybe even not do this, just let the game play on predefined channels which will auto interrup anyways
*/

#include <stdio.h>

#include <SDL2/SDL_mixer.h>

#include "engine.h"

// define the max number of audio channels
#define MAX_CHANNELS 16

// create array to hold audio chunks in memory
Mix_Chunk *pChunks[MAX_CHANNELS] = { NULL };

// counter for total chunks (used in debug)
int totalChunks = 0;

// function to initialize audio system
void initAudio(){
    // opens the mixer to the format specified
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) 
    {
        // catch failure
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }
    // allocate our desired max channels to the mixer
    Mix_AllocateChannels(MAX_CHANNELS);

    // debug: acknowledge audio initialization
    debugOutputComplete(); 
}

// function to free audio chunk from memory by channel
void free_audio_chunk(int channel) {
    // if the channel is invalid
    if (channel < 0 || channel >= MAX_CHANNELS) {
        printf("ERROR: INVALID CHANNEL TO FREE AUDIO CHUNK");
        return; // pass
    }

    // if there is something in the channel we want to free from
    if (pChunks[channel] != NULL) {
        // free the chunk indexed from our chunks at the channel specified
        Mix_FreeChunk(pChunks[channel]);
        
        // set the channel the chunk resided in to NULL
        pChunks[channel] = NULL;
        totalChunks--;
    }
}

// function allowing a sound to be played on a channel by filename
// param "chan" is channel
void playSound(const char *pFilename, int chan, int loops) {
    // open our filename into a chunk
    Mix_Chunk *pSound = Mix_LoadWAV(pFilename);
    
    // if opening failed
    if (pSound == NULL) {
        printf("Error loading audio file: %s\n", Mix_GetError());
        return; // alarm in console and pass
    }
    
    // attempt to play the chunk on the channel,
    // returns which channel it was assigned to
    int channel = Mix_PlayChannel(chan, pSound, loops); 
    
    // if playing failed (assigned channel -1)
    if (channel == -1) {
        printf("Error playing audio file: %s\n", Mix_GetError());
        Mix_FreeChunk(pSound);
        return; // alarm in console, free the allocated chunk and pass
    }

    // if the channel assigned was out of bounds
    if(channel < 0 || channel >= MAX_CHANNELS){
        printf("Error: channel index out of bounds\n");
        Mix_FreeChunk(pSound);
        return; // free the allocated chunk and pass
    }

    // put our channel identifier into the chunks
    pChunks[channel] = pSound;

    // increment total chunks
    totalChunks++;

    // Free audio memory when channel finishes
    Mix_ChannelFinished(free_audio_chunk);
}

// set a specific (or all channels if passed -1) volume level 0-128 
void setVolume(int channel, int volume){
    printf("Setting volume of channel %d to %d.\t",channel,volume);
    Mix_Volume(channel,volume);
    debugOutputComplete();
}

// shut down all audio systems and free all audio chunks
void shutdownAudio(){
    // Halt all playing channels
    Mix_HaltChannel(-1);
    printf("\033[0;31mHalted playing channels.\033[0;37m\n");


    // Free all audio chunks in the chunks array
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (pChunks[i] != NULL) {
            Mix_FreeChunk(pChunks[i]);
            pChunks[i] = NULL;
        }
    }

    // Close the audio mixer
    Mix_CloseAudio();
    printf("\033[0;31mMixer closed.\033[0;37m\n");
}