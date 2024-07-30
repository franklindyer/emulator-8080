#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "cpu.c"
#include "display.c"

#define EXECRATE 1000

char KEYS[322] = {0};
uint8_t clk = 0;

uint8_t handle_sokoban_in(uint8_t port) { 
    if (port == 1) return clk;
    return 0;
}

void handle_sokoban_out(uint8_t port, uint8_t outbyte) {}

void handle_sokoban_events(cpu8080* cpu, arcade_display* display) {
    SDL_Event e;
    update_sokoban_display(display);
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym < 322) {
            KEYS[e.key.keysym.sym] = 1;
        }
        if (e.type == SDL_KEYUP && e.key.keysym.sym < 322) {
            KEYS[e.key.keysym.sym] = 0;
            printf("KEY UP!\n");
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
            if (cpu.pc == 0x0010) {
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
            // cpu.bus = inttype ? 0xcf : 0xd7;
            cpu.bus = 0x00;
        }
    }

    print_cpu_state(&cpu);

    destroy_arcade_display(&display);
}

int main() {
    run_sokoban();
    return 0;
}
