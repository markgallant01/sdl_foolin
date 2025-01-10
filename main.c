#include <SDL2/SDL.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

struct App {
    SDL_Window *window;
    SDL_Surface *windowSurface;
};

bool game_initializeGame(struct App *app);
bool game_loadMedia(void);
void game_close(void);

int main(void)
{
    struct App app;
    game_initializeGame(&app);

    // fill surface white
    SDL_FillRect(app.windowSurface, NULL,
            SDL_MapRGB(app.windowSurface->format, 0xFF, 0xFF, 0xFF));

    // update the surface
    SDL_UpdateWindowSurface(app.window);

    // hack for window to stay up
    SDL_Event e; bool quit = false; while( quit == false ){ while( SDL_PollEvent( &e ) ){ if( e.type == SDL_QUIT ) quit = true; } }

    // destroy window
    SDL_DestroyWindow(app.window);

    // quit subsystems
    SDL_Quit();

    return 0;
}

bool game_initializeGame(struct App *app)
{
    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n",
                SDL_GetError());
        return false;
    }

    // create window
    app->window = SDL_CreateWindow("SDL Tutorial",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (app->window == NULL) {
        fprintf(stderr, "Window could not be created! SDL_ERROR: %s\n",
                SDL_GetError());
        return false;
    }

    // get window surface
    app->windowSurface = SDL_GetWindowSurface(app->window);

    return true;
}
