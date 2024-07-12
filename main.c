#include <SDL2/SDL.h>

#include "screen_drawing.c"

int main() {
    SDL_Window * window = NULL;

    SDL_Surface * window_surface = NULL;
    SDL_Surface * image_surface = NULL;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);

    window_surface = SDL_GetWindowSurface(window);
    image_surface = SDL_LoadBMP("./data/grumpy-cat.bmp");
    SDL_Rect rect = {0,0,10,10};
    char c[5] = {'0', '1', '2', '3', '4'};
    draw_pixel_row(image_surface, c, 8, 0);
    draw_pixel_row(image_surface, 1+c, 8, 1);
    draw_pixel_row(image_surface, 2+c, 8, 2);

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
}
