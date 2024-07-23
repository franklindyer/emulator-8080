#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "screen_drawing.c"

rgb_pixel space_invaders_color_map(int x, int y, int on) {
    rgb_pixel pix = {
        (255 * x * on) / (2 * 256),
        (255 * y * on) / (2 * 224),
        (255 * (2*256 - x) * on) / (2 * 256)
    };
    return pix;
}

void update_space_invaders_display(arcade_display* disp) {
    int i;
    draw_pixel_screen_rotated(disp->image_surface, disp->bitmap, 224, 256, &space_invaders_color_map);
    SDL_BlitSurface(disp->image_surface, NULL, disp->window_surface, NULL);
    SDL_UpdateWindowSurface(disp->window);
}

