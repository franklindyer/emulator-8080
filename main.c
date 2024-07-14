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

void run_invaders() {
    unsigned char* mainmem = malloc(1 << 16);
    cpu8080 cpu = {};
    cpu.memory = mainmem;

    int fd = open("./data/invaders", O_RDONLY);
    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    read(fd, mainmem, size);

    int i = 0;    
    while(i < 500000) {
        i++; printf("%d\t", i);
        emulate_cpu8080(&cpu, 1);
    }
    print_cpu_state(&cpu);
}

int main() {
    run_invaders();
    return 0;
}
