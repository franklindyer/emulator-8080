#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "cpu.c"
#include "display.c"
#include "space_invader_sounds.c"

#define EXECRATE 999999

// All this state external to the CPU should eventually be refactored
int coin_in = 0;
uint8_t shift_amount = 0;
uint16_t shift_register = 0;
char KEYS[322] = {0};
uint8_t dipswitch = 0; // Bits 0,1 control number of lives, bit 2 controls extra life, bit 3 controls coin info
uint8_t port_state[8] = {0};

uint8_t handle_space_invaders_in(uint8_t port) {
    if (port == 0) {
        uint8_t val = 0xe;
        if (KEYS[SDLK_w]) val |= 1 << 4;
        if (KEYS[SDLK_a]) val |= 1 << 5;
        if (KEYS[SDLK_d]) val |= 1 << 6;
        return val;
    }
    if (port == 1) {
        uint8_t val = (uint8_t)coin_in;
        val |= 0x8;
        if (KEYS[SDLK_1]) val |= 1 << 2;
        if (KEYS[SDLK_2]) val |= 1 << 1;
        if (KEYS[SDLK_w]) val |= 1 << 4;
        if (KEYS[SDLK_a]) val |= 1 << 5;
        if (KEYS[SDLK_d]) val |= 1 << 6;
        coin_in = 0;
        return val;
    }
    if (port == 2) {
        uint8_t val = 0xe;
        val |= dipswitch & 3;
        val |= (dipswitch << 1) & 8;
        val |= (dipswitch & 8) << 4;
        if (KEYS[SDLK_w]) val |= 1 << 4;
        if (KEYS[SDLK_a]) val |= 1 << 5;
        if (KEYS[SDLK_d]) val |= 1 << 6;
    }
    if (port == 3) {
        return (shift_register >> (8-shift_amount)) & 0xff;
    }
    return 0;
}

void handle_space_invaders_out(uint8_t port, uint8_t outbyte) {
    if (port == 2) {
        shift_amount = outbyte & 0x7;
    }
    if (port == 3 && outbyte != port_state[3]) {
        if ((outbyte & 1) && !(port_state[3] & 1))
            play_sound(5);
        if ((outbyte & 2) && !(port_state[3] & 2))
            play_sound(0);
        if ((outbyte & 4) && !(port_state[3] & 4))
            play_sound(2);
        if ((outbyte & 8) && !(port_state[3] & 8))
            play_sound(1);
        port_state[3] = outbyte;
    }
    if (port == 4) {
        shift_register = (outbyte << 8) | ((shift_register >> 8) & 0xff);
    }
    if (port == 5) {
        if ((outbyte & 1) && !(port_state[5] & 1))
            play_sound(3);
        if ((outbyte & 2) && !(port_state[5] & 2))
            play_sound(4);
        if ((outbyte & 4) && !(port_state[5] & 4))
            play_sound(3);
        if ((outbyte & 8) && !(port_state[5] & 8))
            play_sound(4);
        port_state[5] = outbyte;
    }
}

void handle_space_invaders_events(cpu8080* cpu, arcade_display* display) {
    SDL_Event e;
    update_space_invaders_display(display);
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym < 322) {
            KEYS[e.key.keysym.sym] = 1;
        }
        if (e.type == SDL_KEYUP && e.key.keysym.sym < 322) {
            KEYS[e.key.keysym.sym] = 0;
            if (e.key.keysym.sym == SDLK_c) {
                coin_in = 1;
            }
        }
        if (e.type == SDL_QUIT) {
            printf("Exiting Space Invaders game...\n");
            exit(0);
        }
    }
}

void run_invaders() {
    unsigned char* mainmem = malloc(1 << 16); 

    cpu8080 cpu = {0}; 
    cpu.memory = mainmem;
    cpu.handle_in = &handle_space_invaders_in;
    cpu.handle_out = &handle_space_invaders_out;

    int fd = open("./space_invaders/invaders", O_RDONLY);
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem, size);

    arcade_display display = init_arcade_display(&mainmem[0x2400], 224, 256, 2);

    printf("Starting Space Invaders game...\n");

    int i = 0;
    int j = 0;
    long k = 0;
    int inttype = 0;
    int step = 0;
    while(1) {
        emulate_cpu8080(&cpu, EXECRATE);
        
        if (cpu.flags.ei) {    
            handle_space_invaders_events(&cpu, &display);
            update_space_invaders_display(&display);
            cpu.flags.i = 1;
            inttype = 1 - inttype;
            cpu.bus = inttype ? 0xcf : 0xd7;
        }
    }

    print_cpu_state(&cpu);

    destroy_arcade_display(&display);
}

int main() {
    run_invaders();
    return 0;
}
