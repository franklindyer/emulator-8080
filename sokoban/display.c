#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "screen_drawing.c"

rgb_pixel sokoban_color_map(int x, int y, int on) {
    rgb_pixel pix = {
        255*on,
        255*on,
        255*on
    };
    return pix;
}

void update_sokoban_display(arcade_display* disp) {
    draw_pixel_screen(disp->image_surface, disp->bitmap, 256, 256, &sokoban_color_map);
    SDL_BlitSurface(disp->image_surface, NULL, disp->window_surface, NULL);
    SDL_UpdateWindowSurface(disp->window);
}

