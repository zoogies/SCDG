#ifndef AUDIO_H
#define AUDIO_H

// initialize audio components for engine
void initAudio();

// play a sound by its filename path and specify number of loops (-1 for looping)
void playSound(const char *filename, int loops);

// shut down all audio systems and free all memory assosciated
void shutdownAudio();

#endif