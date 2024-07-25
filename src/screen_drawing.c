#include <SDL2/SDL.h>

typedef struct rgb_pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_pixel;

void draw_pixel_screen(
        SDL_Surface* screen, 
        unsigned char* bitmap, 
        int r, int c,
        rgb_pixel (*color_map)(int x, int y, int on)) {
    int pix_w = screen->w / r;
    int pix_h = screen->h / c;
    rgb_pixel fill;
    SDL_Rect rect = {0, 0, pix_w, pix_h};
    for (int i = 0; i < r*c; i++) {
        int rem = i % 8; // (7 - i % 8);
        fill = (*color_map)(rect.x, rect.y, (bitmap[i/8] & (1 << rem)) >> rem);
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, fill.r, fill.g, fill.b));
        rect.x += pix_w;
        if (rect.x >= screen->w) {
            rect.x = 0;
            rect.y += pix_h;
        }
    }
}

void draw_pixel_screen_rotated(
        SDL_Surface* screen, 
        unsigned char* bitmap, 
        int r, int c,
        rgb_pixel (*color_map)(int x, int y, int on)) {
    int pix_w = screen->w / r;
    int pix_h = screen->h / c;
    rgb_pixel fill;
    SDL_Rect rect = {0, screen->h - pix_h, pix_w, pix_h};
    for (int i = 0; i < r*c; i++) {
        int rem = i % 8;
        fill = (*color_map)(rect.x, rect.y, (bitmap[i/8] & (1 << rem)) >> rem);
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, fill.r, fill.g, fill.b));
        rect.y += -pix_h;
        if (rect.y < 0) {
            rect.y = screen->h - pix_h;
            rect.x += pix_w;
        }
    }
}

typedef struct arcade_display {
    SDL_Window* window;
    SDL_Surface* window_surface;
    SDL_Surface* image_surface;
    unsigned char* bitmap;
} arcade_display;

arcade_display init_arcade_display(unsigned char* vidmem, int w, int h, int pixsize) {
    arcade_display disp = {};
    SDL_Init(SDL_INIT_VIDEO);
    disp.window = SDL_CreateWindow(
        "Window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        w*pixsize, h*pixsize,
        SDL_WINDOW_SHOWN
    );

    disp.window_surface = SDL_GetWindowSurface(disp.window);
    disp.image_surface = SDL_CreateRGBSurface(
        SDL_SWSURFACE,
        w*pixsize, h*pixsize, 32,
        0xff << 24, 0xff << 16, 0xff << 8, 0xff
    );
    disp.bitmap = vidmem;

    return disp;
}

void destroy_arcade_display(arcade_display* disp) {
    SDL_DestroyWindow(disp->window);
    SDL_Quit();
}
