#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "cpu.c"
#include "display.c"

#define EXECRATE 999999

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
        }
        if (e.type == SDL_QUIT) {
            printf("Exiting Lunar Rescue game...\n");
            exit(0);
        }
    }
}



uint8_t handle_lunar_rescue_in(uint8_t port) { return 0; }
void handle_lunar_rescue_out(uint8_t port, uint8_t outbyte) { return; }

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
