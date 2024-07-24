#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SHOOT_SOUND 0
#define ALIENKILLED_SOUND 1
#define THRUST_SOUND 2
#define DIED_SOUND 3
#define STEP1_SOUND 4
#define STEP2_SOUND 5

void play_sound(int sound_code) {
    switch(sound_code) {
        case SHOOT_SOUND:
            system("afplay ./lunar_rescue/sounds/beamgun.wav &");
            break;
        case ALIENKILLED_SOUND:
            system("afplay ./lunar_rescue/sounds/alienexplosion.wav &");
            break;
        case THRUST_SOUND:
            system("afplay --rate 2 ./lunar_rescue/sounds/thrust.wav &");
            break;
        case DIED_SOUND:
            system("afplay ./lunar_rescue/sounds/rescueshipexplosion.wav &");
            break;
        case STEP1_SOUND:
            system("afplay ./lunar_rescue/sounds/steph.wav &");
            break;
        case STEP2_SOUND:
            system("afplay ./lunar_rescue/sounds/stepl.wav &");
            break;
        default:
            break;
    }
}
