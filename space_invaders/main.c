#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "cpu.c"
#include "screen_drawing.c"

#define EXECRATE 999999

// All this state external to the CPU should eventually be refactored
int coin_in = 0;
uint8_t shift_amount = 0;
uint16_t shift_register = 0;
char KEYS[322] = {0};

typedef struct space_invaders_display {
    SDL_Window* window;
    SDL_Surface* window_surface;
    SDL_Surface* image_surface;
    unsigned char* bitmap;
} space_invaders_display;

space_invaders_display init_space_invaders_display(unsigned char* vidmem) {
    space_invaders_display disp = {};
    SDL_Init(SDL_INIT_VIDEO);
    disp.window = SDL_CreateWindow(
        "Window", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        224*2, 256*2, 
        SDL_WINDOW_SHOWN
    );

    disp.window_surface = SDL_GetWindowSurface(disp.window);
    disp.image_surface = SDL_CreateRGBSurface(
        SDL_SWSURFACE, 
        224*2, 256*2, 32, 
        0xff << 24, 0xff << 16, 0xff << 8, 0xff
    );
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
        if (KEYS[SDLK_1]) val |= 1 << 2;
        if (KEYS[SDLK_2]) val |= 1 << 1;
        if (KEYS[SDLK_w]) val |= 1 << 4;
        if (KEYS[SDLK_a]) val |= 1 << 5;
        if (KEYS[SDLK_d]) val |= 1 << 6;
        coin_in = 0;
        return val;
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
    if (port == 4) {
        shift_register = (outbyte << 8) | ((shift_register >> 8) & 0xff);
    }
}

void handle_space_invaders_events(cpu8080* cpu, space_invaders_display* display) {
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

    int fd = open("./data/invaders", O_RDONLY);
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem, size);

    space_invaders_display display = init_space_invaders_display(&mainmem[0x2400]);

    printf("Starting Space Invaders game...\n");

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
            if (cpu.pc == 0xffff) {
                uint16_t pc = cpu.pc;
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
            update_space_invaders_display(&display);
            cpu.flags.i = 1;
            inttype = 1 - inttype;
            cpu.bus = inttype ? 0xcf : 0xd7;
        }
    }

    print_cpu_state(&cpu);

    destroy_space_invaders_display(&display);
}

int main() {
    run_invaders();
    return 0;
}
