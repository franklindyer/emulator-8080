#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SHOOT_SOUND 0
#define INVADERKILLED_SOUND 1
#define EXPLOSION_SOUND 2
#define INVADERMOVE1_SOUND 3
#define INVADERMOVE2_SOUND 4
#define UFO_SOUND 5

void play_sound(int sound_code) {
    switch(sound_code) {
        case SHOOT_SOUND:
            system("afplay ./space_invaders/sounds/shoot.wav &");
            break;
        case INVADERKILLED_SOUND:
            system("afplay ./space_invaders/sounds/invaderkilled.wav &");
            break;
        case EXPLOSION_SOUND:
            system("afplay ./space_invaders/sounds/explosion.wav &");
            break;
        case INVADERMOVE1_SOUND:
            system("afplay ./space_invaders/sounds/fastinvader1.wav &");
            break;
        case INVADERMOVE2_SOUND:
            system("afplay ./space_invaders/sounds/fastinvader2.wav &");
            break;
        case UFO_SOUND:
            system("afplay ./space_invaders/sounds/ufo_lowpitch.wav &");
            break;
        default:
            break;
    }
}
