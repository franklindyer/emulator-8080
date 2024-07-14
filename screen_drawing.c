#include <SDL2/SDL.h>

void draw_pixel_row(SDL_Surface* screen, unsigned char* bitmap, int n, int r) {
    int pix_w = screen->w / n;
    int pix_h = screen->h / n;
    int fill = 0;
    SDL_Rect rect = {0, r*pix_h, pix_w, pix_h};
    for (int i = 0; i < n; i++) {
        int rem = (7 - i % 8);
        fill = 255 * ((bitmap[i/8] & (1 << rem)) >> rem); fill = 255 - fill;
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, fill, fill, fill)); 
        rect.x += pix_w;
    }
}

void draw_pixel_screen(SDL_Surface* screen, unsigned char* bitmap, int r, int c) {
    int pix_w = screen->w / c;
    int pix_h = screen->h / r;
    int fill = 0;
    SDL_Rect rect = {0, 0, pix_w, pix_h};
    for (int i = 0; i < r*c; i++) {
        int rem = (7 - i % 8);
        fill = 255 * ((bitmap[i/8] & (1 << rem)) >> rem); fill = 255 - fill;
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, fill, fill, fill));
        rect.x += pix_w;
        if (rect.x >= screen->w) {
            rect.x = 0;
            rect.y += pix_h;
        }
    }
}

void draw_pixel_screen_rotated(SDL_Surface* screen, unsigned char* bitmap, int r, int c) {
    int pix_w = screen->w / r;
    int pix_h = screen->h / c;
    int fill = 0;
    SDL_Rect rect = {0, screen->h - pix_h, pix_w, pix_h};
    for (int i = 0; i < r*c; i++) {
        int rem = (7 - i % 8);
        fill = 255 * ((bitmap[i/8] & (1 << rem)) >> rem); fill = 255 - fill;
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, fill, fill, fill));
        rect.y += -pix_h;
        if (rect.y < 0) {
            rect.y = screen->h - pix_h;
            rect.x += pix_w;
        }
    }
}
