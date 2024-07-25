#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "screen_drawing.c"

typedef struct gunfight_display {
    SDL_Window* window;
    SDL_Surface* window_surface;
    SDL_Surface* image_surface;
    unsigned char* bitmap;
} gunfight_display;

gunfight_display init_gunfight_display(unsigned char* vidmem) {
    gunfight_display disp = {};
    SDL_Init(SDL_INIT_VIDEO);
    disp.window = SDL_CreateWindow(
        "Window", 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        256*2, 224*2, 
        SDL_WINDOW_SHOWN
    );

    disp.window_surface = SDL_GetWindowSurface(disp.window);
    disp.image_surface = SDL_CreateRGBSurface(
        SDL_SWSURFACE, 
        256*2, 224*2, 32, 
        0xff << 24, 0xff << 16, 0xff << 8, 0xff
    );
    disp.bitmap = vidmem;

    return disp;
}

rgb_pixel gunfight_color_map(int x, int y, int on) {
    rgb_pixel pix = { 255*on, 255*on, 0 };
    return pix;
}

void update_gunfight_display(gunfight_display* disp) {
    int i;
    draw_pixel_screen(disp->image_surface, disp->bitmap, 256, 224, &gunfight_color_map);
    SDL_BlitSurface(disp->image_surface, NULL, disp->window_surface, NULL);
    SDL_UpdateWindowSurface(disp->window);
}

void destroy_gunfight_display(gunfight_display* disp) {
    SDL_DestroyWindow(disp->window);
    SDL_Quit();
}

