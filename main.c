#include <SDL2/SDL.h>

int main() {
    SDL_Window * window = NULL;

    SDL_Surface * window_surface = NULL;
    SDL_Surface * image_surface = NULL;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);

    window_surface = SDL_GetWindowSurface(window);
    image_surface = SDL_LoadBMP("./data/grumpy-cat.bmp");
    SDL_Rect rect = {0,0,100,100};
    SDL_FillRect(image_surface, &rect, SDL_MapRGB(image_surface->format, 0x00, 0x00, 0x00));

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
