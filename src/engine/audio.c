#include <stdio.h>

#include <SDL2/SDL_mixer.h>

#include "engine.h"

#define MAX_CHANNELS 16

Mix_Chunk *chunks[MAX_CHANNELS] = { NULL };

// TODO volume control

void initAudio(){
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }
  
    Mix_AllocateChannels(MAX_CHANNELS);
    debugOutputComplete(); // debug: acknowledge audio initialization
}

void free_audio_chunk(int channel) {
    if (channel < 0 || channel >= MAX_CHANNELS) {
        return;
    }

    if (chunks[channel] != NULL) {
        Mix_FreeChunk(chunks[channel]);
        chunks[channel] = NULL;
    }
}

void playSound(const char *filename, int loops) {
    Mix_Chunk *sound = Mix_LoadWAV(filename);
    
    if (sound == NULL) {
        printf("Error loading audio file: %s\n", Mix_GetError());
        return;
    }
    
    int channel = Mix_PlayChannel(-1, sound, loops);
    if (channel == -1) {
        printf("Error playing audio file: %s\n", Mix_GetError());
        Mix_FreeChunk(sound);
        return;
    }

    if(channel < 0 || channel >= MAX_CHANNELS){
        printf("Error: channel index out of bounds\n");
        Mix_FreeChunk(sound);
        return; 
    }

    chunks[channel] = sound; // TODO: this seems like it would break if previous if catch errors?

    // Free audio memory when channel finishes
    Mix_ChannelFinished(free_audio_chunk);
}

void shutdownAudio(){
    // Halt all playing channels
    Mix_HaltChannel(-1);

    // Free all audio chunks in the chunks array
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (chunks[i] != NULL) {
            Mix_FreeChunk(chunks[i]);
            chunks[i] = NULL;
        }
    }

    // Close the audio system
    Mix_CloseAudio();
}