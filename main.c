#include <SDL2/SDL.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(void)
{
    // render window
    SDL_Window *window = NULL;

    // window surface
    SDL_Surface *screenSurface = NULL;

    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n",
                SDL_GetError());
    } else {
        // create window
        window = SDL_CreateWindow("SDL Tutorial",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL) {
            printf("Window could not be created! SDL_ERROR: %s\n",
                    SDL_GetError());
        } else {
            // get window surface
            screenSurface = SDL_GetWindowSurface(window);

            // fill surface white
            SDL_FillRect(screenSurface, NULL,
                    SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

            // update the surface
            SDL_UpdateWindowSurface(window);

            // hack for window to stay up
            SDL_Event e; bool quit = false; while( quit == false ){ while( SDL_PollEvent( &e ) ){ if( e.type == SDL_QUIT ) quit = true; } }
        }
    }

    // destroy window
    SDL_DestroyWindow(window);

    // quit subsystems
    SDL_Quit();

    return 0;
}
