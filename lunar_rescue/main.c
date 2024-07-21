#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "cpu.c"
#include "display.c"

#define EXECRATE 999999

int coin_in = 0;
uint8_t shift_amount = 0;
uint16_t shift_register = 0;
int KEYS[322];

void load_game_into_memory(unsigned char* mainmem) {
    int fd = open("./lunar_rescue/lrescue.1", O_RDONLY);
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem, size);
    close(fd);

    fd = open("./lunar_rescue/lrescue.2", O_RDONLY);
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem+0x800, size);
    close(fd);

    fd = open("./lunar_rescue/lrescue.3", O_RDONLY);
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem+0x1000, size);
    close(fd);

    fd = open("./lunar_rescue/lrescue.4", O_RDONLY);
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem+0x1800, size);
    close(fd);

    fd = open("./lunar_rescue/lrescue.5", O_RDONLY);
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem+0x4000, size);
    close(fd);

    fd = open("./lunar_rescue/lrescue.6", O_RDONLY);
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem+0x4800, size);
    close(fd);
}

void handle_lunar_rescue_events(cpu8080* cpu, lunar_rescue_display* display) {
    SDL_Event e;
    update_lunar_rescue_display(display);
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym < 322) {
            KEYS[e.key.keysym.sym] = 1;
        }
        if (e.type == SDL_KEYUP && e.key.keysym.sym < 322) {
            KEYS[e.key.keysym.sym] = 0;
            if (e.key.keysym.sym == SDLK_c) coin_in = 1;
        }
        if (e.type == SDL_QUIT) {
            printf("Exiting Lunar Rescue game...\n");
            exit(0);
        }
    }
}

// PORT 1, BIT 0 -> Coin inserted
// PORT 1, BIT 2 -> Play 1-player
// PORT 1, BIT 3 -> Play 2-player
// PORT 1, BIT 4 -> Open carrier door, shoot
// PORT 1, BIT 5 -> Move left
// PORT 1, BIT 6 -> Move right
// PORT 2, BIT 2 -> TILT pin
// PORT 3        -> Hardware shift register input
uint8_t foo = 0xff;
uint8_t handle_lunar_rescue_in(uint8_t port) {
    // if (port == 2) return 0xfb;
    if (port == 1) {
        uint8_t val = (uint8_t)coin_in;
        if (KEYS[SDLK_1]) val |= 1 << 2;
        if (KEYS[SDLK_2]) val |= 1 << 3;
        if (KEYS[SDLK_w]) val |= 1 << 4;
        if (KEYS[SDLK_a]) val |= 1 << 5;
        if (KEYS[SDLK_d]) val |= 1 << 6;
        coin_in = 0;
        return val;
    }
    if (port == 2) return 0x00; // 0xfb
    if (port == 3) {
        return (shift_register >> (8-shift_amount)) & 0xff;
    }
    return 0; 
}

void handle_lunar_rescue_out(uint8_t port, uint8_t outbyte) {
    if (port == 2) {
        shift_amount = outbyte & 0x7;
    }
    if (port == 4) {
        shift_register = (outbyte << 8) | ((shift_register >> 8) & 0xff);
    }
}

void run_lrescue() {
    unsigned char* mainmem = malloc(1 << 16); 

    cpu8080 cpu = {0}; 
    cpu.memory = mainmem;
    cpu.handle_in = &handle_lunar_rescue_in;
    cpu.handle_out = &handle_lunar_rescue_out;

    load_game_into_memory(mainmem);

    lunar_rescue_display display = init_lunar_rescue_display(&mainmem[0x2400]);

    printf("Starting Lunar Rescue game...\n");
    fflush(stdout);

    int i = 0;
    int j = 0;
    long k = 0;
    int inttype = 0;
    int step = 0;
    while(1) {
        j = 0;
        while (j < EXECRATE) {
            j++; k++;
            emulate_cpu8080(&cpu, 1);
            if (0) {
                uint16_t pc = cpu.pc;
                printf("Counter is: %ld\n", k);
                step = 1;
            }
            if (step) {
                print_cpu_state(&cpu); 
                update_lunar_rescue_display(&display);
                if (getchar() == 'c') step = 0;
            }
        }
        // emulate_cpu8080(&cpu, EXECRATE);
        
        if (cpu.flags.ei) {    
            handle_lunar_rescue_events(&cpu, &display);
            update_lunar_rescue_display(&display);
            cpu.flags.i = 1;
            inttype = 1 - inttype;
            cpu.bus = inttype ? 0xcf : 0xd7;
        }
    }

    print_cpu_state(&cpu);

    destroy_lunar_rescue_display(&display);
}

int main() {
    run_lrescue();
    return 0;
}
