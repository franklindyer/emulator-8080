#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "screen_drawing.c"

rgb_pixel lunar_rescue_color_map(int x, int y, int on) {
    int r, g, b;
    if (y/2 >= (256 - 12)) {
        r = 0; g = 255 * on; b = 0;
    } else if (y/2 >= (256 - 28)) {
        r = (255 * ((256-12) - y/2) * on) / 16;
        g = 255 * on;
        b = 0; 
    } else if (y/2 >= (256 - 36)) { 
        r = 255 * on;
        g = (255 * (y/2 - (256-36)) * on) / 8;
        b = (255 * ((256-28) - y/2) * on) / 8;
    } else if (y/2 >= (256 - 44)) {
        r = (255 * (y/2 - (256-44)) * on) / 8;
        g = (255 * ((256-36) - y/2) * on) / 8;
        b = 255 * on;
    } else {
        r = 0;
        g = (255 * (y/2 - (256 - 44)) * on) / 220;
        b = 255 * on;
    }
    rgb_pixel pix = { r, g, b };
    return pix;
}

void update_lunar_rescue_display(arcade_display* disp) {
    int i;
    draw_pixel_screen_rotated(disp->image_surface, disp->bitmap, 224, 256, &lunar_rescue_color_map);
    SDL_BlitSurface(disp->image_surface, NULL, disp->window_surface, NULL);
    SDL_UpdateWindowSurface(disp->window);
}

