#include <SDL2/SDL.h>

void draw_pixel_row(SDL_Surface* screen, char* bitmap, int n, int r) {
    int pix_w = screen->w / n;
    int pix_h = screen->h / n;
    int fill = 0;
    SDL_Rect rect = {0, r*pix_h, pix_w, pix_h};
    for (int i = 0; i < n; i++) {
        int rem = (7 - i % 8);
        fill = 255 * ((bitmap[i/8] & (1 << rem)) >> rem);
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, fill, fill, fill)); 
        rect.x += pix_w;
    }
}
