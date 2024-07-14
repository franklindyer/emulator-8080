#include <fcntl.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "cpu.c"
#include "screen_drawing.c"

int screen_test() {
    SDL_Window * window = NULL;

    SDL_Surface * window_surface = NULL;
    SDL_Surface * image_surface = NULL;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);

    window_surface = SDL_GetWindowSurface(window);
    image_surface = SDL_LoadBMP("./data/grumpy-cat.bmp");
    SDL_Rect rect = {0,0,10,10};
    char c[5] = {'0', '1', '2', '3', '4'};
    draw_pixel_row(image_surface, c, 40, 0);

    SDL_BlitSurface(image_surface, NULL, window_surface, NULL);

    SDL_UpdateWindowSurface(window);

    SDL_Event e;
    int quit = 0;
    while (!quit){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = 1;
            }
            if (e.type == SDL_KEYDOWN){
                quit = 1;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_FreeSurface(image_surface);
    SDL_Quit();

    return 0;
}

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
    while(i < 500000) {
        i++; printf("%d\t", i);
        emulate_cpu8080(&cpu, 1);
        if (i % 10000 == 0) { handle_space_invaders_events(&cpu, &display);
        getchar(); }
    }
    print_cpu_state(&cpu);

    destroy_space_invaders_display(&display);
}

int main() {
    run_invaders();
    return 0;
}
