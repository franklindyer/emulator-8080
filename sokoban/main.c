#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "cpu.c"
#include "display.c"
#include "sokoban_sounds.c"

#define EXECRATE 100000

char KEYS[322] = {0};
uint8_t clk = 0;
uint8_t port_in_state[8];

uint8_t handle_sokoban_in(uint8_t port) { 
    if (port == 0) {
        uint8_t keymap = 
            KEYS[SDLK_w] 
            | (KEYS[SDLK_s] << 1) 
            | (KEYS[SDLK_a] << 2) 
            | (KEYS[SDLK_d] << 3);
        KEYS[SDLK_w] = 0;
        KEYS[SDLK_a] = 0;
        KEYS[SDLK_s] = 0;
        KEYS[SDLK_d] = 0;
        return keymap;
    }
    if (port == 1) {
        uint8_t keymap = KEYS[SDLK_r];
        KEYS[SDLK_r] = 0;
        return keymap;
    };
    return 0;
}

void handle_sokoban_out(uint8_t port, uint8_t outbyte) {
    if (port == 1) {
        if (outbyte & 2) play_sound(0);         // NOPE
        else if (outbyte & 4) play_sound(1);    // CRATE PUSH
        else if (outbyte & 1) play_sound(2);    // WALKING
        else if (outbyte & 8) play_sound(3);    // WIN LEVEL
        else if (outbyte & 128) play_sound(7);  // STOP ALL SOUNDS
    }
}

void handle_sokoban_events(cpu8080* cpu, arcade_display* display) {
    SDL_Event e;
    update_sokoban_display(display);
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    while (SDL_PollEvent(&e)) {
//        if (e.type == SDL_KEYDOWN && e.key.keysym.sym < 322) {
//            KEYS[e.key.keysym.sym] = 1;
//        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym < 322) {
            KEYS[e.key.keysym.sym] = 1;
        }
        if (e.type == SDL_QUIT) {
            printf("Exiting Sokoban game...\n");
            exit(0);
        }
    }
}

void load_sokoban_rom(unsigned char* mainmem) {
    int fd = open("./sokoban/rom/sokoban-isrs", O_RDONLY);
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem, size);
    close(fd); 
    
    fd = open("./sokoban/rom/sokoban-code", O_RDONLY);
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem+0x40, size);
    close(fd); 
    
    fd = open("./sokoban/rom/sokoban-variables", O_RDONLY);
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem+0x1d00, size);
    close(fd); 
    
    fd = open("./sokoban/rom/sokoban-sprites", O_RDONLY);
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem+0x2000, size);
    close(fd);
    
    fd = open("./sokoban/rom/sokoban-data", O_RDONLY);
    size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem+0x5000, size);
    close(fd); 
}

void run_sokoban() {
    uint8_t* mainmem = calloc(1 << 16, 1); 

    cpu8080 cpu = {0}; 
    cpu.memory = mainmem;
    cpu.handle_in = &handle_sokoban_in;
    cpu.handle_out = &handle_sokoban_out;
    cpu.flags.ei = 1;
    cpu.sp = 0x2fff;

    load_sokoban_rom(mainmem);

    arcade_display display = init_arcade_display(&mainmem[0x3000], 256, 256, 3);

    printf("Starting Sokoban game...\n");

    int i = 0;
    int j = 0;
    long k = 0;
    int inttype = 0;
    int step = 0;
    while(1) {
        k++;
        j = 0;
        while (j < EXECRATE) {
            j++;
            emulate_cpu8080(&cpu, 1);
            if (cpu.pc == 0xffff) {
                uint16_t pc = cpu.pc;
                printf("Counter is: %ld\n", k);
                step = 1;
            }
            if (step) {
                print_cpu_state(&cpu);
                update_sokoban_display(&display);
                if (getchar() == 'c') step = 0;
            }
        }
        // emulate_cpu8080(&cpu, EXECRATE);
        
        clk++;
        if (cpu.flags.ei) {
            update_sokoban_display(&display); 
            handle_sokoban_events(&cpu, &display);
            cpu.flags.i = 1;
            inttype = 1 - inttype;
            cpu.bus = 0xcf;
            // cpu.bus = 0x00;
        }
    }

    print_cpu_state(&cpu);

    destroy_arcade_display(&display);
}

int main() {
    run_sokoban();
    return 0;
}
