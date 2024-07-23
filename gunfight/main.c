#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "cpu.c"
#include "display.c"

#define EXECRATE 999999

// All this state external to the CPU should eventually be refactored
int coin_in = 0;
uint8_t shift_amount = 0;
uint16_t shift_register = 0;
char KEYS[322] = {0};
uint8_t dipswitch = 0; // Bits 0,1 control number of lives, bit 2 controls extra life, bit 3 controls coin info
uint8_t port_state[8] = {0};
uint8_t left_cowboy_arm = 0x0;
uint8_t right_cowboy_arm = 0x0;

// PORT 0 -> 8 bits of left cowboy data
// PORT 1 -> 8 bits of right cowboy data
// PORT 2 -> Dipswitches and coin detection
// PORT 2, BIT 6    -> Coin inserted
// PORT 3 -> Hardware shift register output
uint8_t handle_gunfight_in(uint8_t port) {
    if (port == 0) {
        uint8_t val = 0x0;
        switch(left_cowboy_arm) {
            case 0: val = 3 << 4; break;
            case 1: val = 7 << 4; break;
            case 2: val = 5 << 4; break;
            case 3: val = 1 << 4; break;
            case 4: val = 4 << 4; break;
            case 5: val = 6 << 4; break;
            case 6: val = 2 << 4; break;
            default: break;
        }
        if (KEYS[SDLK_w]) val |= 1 << 0;
        if (KEYS[SDLK_s]) val |= 1 << 1;
        if (KEYS[SDLK_a]) val |= 1 << 2;
        if (KEYS[SDLK_d]) val |= 1 << 3;
        if (KEYS[SDLK_q]) val |= 1 << 7;
        return val;
    }
    if (port == 1) {
        uint8_t val = 0x0;
        switch(right_cowboy_arm) {
            case 0: val = 3 << 4; break;
            case 1: val = 7 << 4; break;
            case 2: val = 5 << 4; break;
            case 3: val = 1 << 4; break;
            case 4: val = 4 << 4; break;
            case 5: val = 6 << 4; break;
            case 6: val = 2 << 4; break;
            default: break;
        }
        if (KEYS[SDLK_i]) val |= 1 << 0;
        if (KEYS[SDLK_k]) val |= 1 << 1;
        if (KEYS[SDLK_j]) val |= 1 << 2;
        if (KEYS[SDLK_l]) val |= 1 << 3;
        if (KEYS[SDLK_u]) val |= 1 << 7;
        return val;
    }
    if (port == 2) {
        uint8_t val = 0x0;
        if (coin_in) {
            coin_in += -1;
            val |= 1 << 6;
        }
        return val;
    }
    if (port == 3) {
        return (shift_register >> (8-shift_amount)) & 0xff;
    }
    return 0;
}

void handle_gunfight_out(uint8_t port, uint8_t outbyte) {
    if (port == 2) {
        shift_amount = outbyte & 0x7;
    }
    if (port == 3 && outbyte != port_state[3]) {
        // if ((outbyte & 1) && !(port_state[3] & 1))
            // play_sound(5);
        // if ((outbyte & 2) && !(port_state[3] & 2))
            // play_sound(0);
        // if ((outbyte & 4) && !(port_state[3] & 4))
            // play_sound(2);
        // if ((outbyte & 8) && !(port_state[3] & 8))
            // play_sound(1);
        port_state[3] = outbyte;
    }
    if (port == 4) {
        shift_register = (outbyte << 8) | ((shift_register >> 8) & 0xff);
    }
    if (port == 5) {
        /* if ((outbyte & 1) && !(port_state[5] & 1))
            play_sound(3);
        if ((outbyte & 2) && !(port_state[5] & 2))
            play_sound(4);
        if ((outbyte & 4) && !(port_state[5] & 4))
            play_sound(3);
        if ((outbyte & 8) && !(port_state[5] & 8))
            play_sound(4); */
        port_state[5] = outbyte;
    }
}

void handle_gunfight_events(cpu8080* cpu, gunfight_display* display) {
    SDL_Event e;
    update_gunfight_display(display);
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym < 322) {
            KEYS[e.key.keysym.sym] = 1;
        }
        if (e.type == SDL_KEYUP && e.key.keysym.sym < 322) {
            KEYS[e.key.keysym.sym] = 0;
            if (e.key.keysym.sym == SDLK_c) {
                coin_in = 2;
            }
            if (e.key.keysym.sym == SDLK_e) left_cowboy_arm = (left_cowboy_arm + 1) % 7;
            if (e.key.keysym.sym == SDLK_o) right_cowboy_arm = (right_cowboy_arm + 1) % 7;
        }
        if (e.type == SDL_QUIT) {
            printf("Exiting Gunfight game...\n");
            exit(0);
        }
    }
}

void run_invaders() {
    unsigned char* mainmem = malloc(1 << 16); 

    cpu8080 cpu = {0}; 
    cpu.memory = mainmem;
    cpu.handle_in = &handle_gunfight_in;
    cpu.handle_out = &handle_gunfight_out;

    int fd = open("./gunfight/gunfight", O_RDONLY);
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem, size);

    gunfight_display display = init_gunfight_display(&mainmem[0x2400]);

    printf("Starting Gunfight game...\n");

    int i = 0;
    int j = 0;
    long k = 0;
    int inttype = 0;
    int step = 0;
    while(1) {
        // FOR DEBUGGING ONLY
        /* j = 0;
        while (j < EXECRATE) {
            j++; k++;
            emulate_cpu8080(&cpu, 1);
            if (cpu.pc == 0xffff) {
                uint16_t pc = cpu.pc;
                printf("Counter is: %ld\n", k);
                step = 1;
            }
            if (step) {
                print_cpu_state(&cpu); 
                update_gunfight_display(&display);
                if (getchar() == 'c') step = 0;
            }
        } */
        emulate_cpu8080(&cpu, EXECRATE);
        
        if (cpu.flags.ei) {    
            handle_gunfight_events(&cpu, &display);
            update_gunfight_display(&display);
            cpu.flags.i = 1;
            inttype = 1 - inttype;
            cpu.bus = inttype ? 0xcf : 0xd7;
        }
    }

    print_cpu_state(&cpu);

    destroy_gunfight_display(&display);
}

int main() {
    run_invaders();
    return 0;
}
