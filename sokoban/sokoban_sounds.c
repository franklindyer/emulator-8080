#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define NOPE_SOUND 0
#define PUSH_SOUND 1
#define STEP_SOUND 2

void play_sound(int sound_code) {
    switch(sound_code) {
        case NOPE_SOUND:
            system("afplay ./sokoban/sounds/no-movement.mp3 &");
            break;
        case PUSH_SOUND:
            system("afplay ./sokoban/sounds/box-moved.mp3 &");
            break;
        case STEP_SOUND:
            system("afplay ./sokoban/sounds/footsteps.m4a &");
            break;
        default:
            break;
    }
}
