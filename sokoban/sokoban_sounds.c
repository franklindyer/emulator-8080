#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define NOPE_SOUND 0
#define PUSH_SOUND 1
#define STEP_SOUND 2
#define WIN_SOUND 3
#define END_SOUNDS 7

void play_sound(int sound_code) {
    switch(sound_code) {
        case NOPE_SOUND:
            system("afplay ./sokoban/sounds/no-movement.mp3 &");
            break;
        case PUSH_SOUND:
            system("afplay ./sokoban/sounds/box-slide.wav &");
            break;
        case STEP_SOUND:
            system("afplay ./sokoban/sounds/footsteps.m4a &");
            break;
        case WIN_SOUND:
            system("afplay ./sokoban/sounds/youwin.mp3 &");
            break;
        case END_SOUNDS:
            system("killall afplay &");
            break;
        default:
            break;
    }
}
