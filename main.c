#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "cpu.c"
#include "screen_drawing.c"

#define EXECRATE 999999

typedef struct space_invaders_display {
    SDL_Window* window;
    SDL_Surface* window_surface;
    SDL_Surface* image_surface;
    unsigned char* bitmap;
} space_invaders_display;

space_invaders_display init_space_invaders_display(unsigned char* vidmem) {
    space_invaders_display disp = {};
    SDL_Init(SDL_INIT_VIDEO);
    disp.window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 224*2, 256*2, SDL_WINDOW_SHOWN);

    disp.window_surface = SDL_GetWindowSurface(disp.window);
    disp.image_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 224*2, 256*2, 32, 0xff << 24, 0xff << 16, 0xff << 8, 0xff);
    disp.bitmap = vidmem;

    return disp;
}

void update_space_invaders_display(space_invaders_display* disp) {
    int i;
    draw_pixel_screen_rotated(disp->image_surface, disp->bitmap, 224, 256);
    SDL_BlitSurface(disp->image_surface, NULL, disp->window_surface, NULL);
    SDL_UpdateWindowSurface(disp->window);
}

void destroy_space_invaders_display(space_invaders_display* disp) {
    SDL_DestroyWindow(disp->window);
    SDL_Quit();
}

void handle_space_invaders_events(cpu8080* cpu, space_invaders_display* display) {
    SDL_Event e;
    update_space_invaders_display(display);
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_MOUSEBUTTONDOWN) printf("Clicky clicky!\n");
    }
}

void run_invaders() {
    unsigned char* mainmem = malloc(1 << 16); 
    cpu8080 cpu = {}; 
    cpu.memory = mainmem;

    int fd = open("./data/invaders", O_RDONLY);
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem, size);

    space_invaders_display display = init_space_invaders_display(&mainmem[0x2400]);

    int i = 0;
    int j = 0;
    long k = 0;
    int inttype = 0;
    int step = 0;
    while(1) {
        j = 0;
        while (j < EXECRATE) {
            j++; k++; // printf("%d\t", j);
            emulate_cpu8080(&cpu, 1);
//            if ((cpu.pc == 0x015a1) && (k >= 1228998600)) {
            if (cpu.pc == 0xffff) {
                uint16_t pc = cpu.pc;
                printf("INITIAL DY FOR REF ALIEN IS: %02x\n", cpu.memory[0x2007]);
                printf("Counter is: %ld\n", k);
                step = 1;
            }
            if (step) {
                print_cpu_state(&cpu); 
                update_space_invaders_display(&display);
                if (getchar() == 'c') step = 0;
            }
        }
        
        if (cpu.flags.ei) {    
        handle_space_invaders_events(&cpu, &display);
        // usleep(8333);
        update_space_invaders_display(&display);
        cpu.flags.i = 1;
        inttype = 1 - inttype;
        cpu.bus = inttype ? 0xcf : 0xd7;
        // usleep(1000000);
        }
    }

    print_cpu_state(&cpu);

    destroy_space_invaders_display(&display);
}

int main() {
    run_invaders();
    return 0;
}
