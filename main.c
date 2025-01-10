#include <SDL2/SDL.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

struct App {
    SDL_Window *window;
    SDL_Surface *windowSurface;
};

bool game_initializeGame(struct App *app);
SDL_Surface *game_loadMedia(void);
void game_close(struct App *app);

int main(void)
{
    struct App app;
    if (!game_initializeGame(&app)) {
        return EXIT_FAILURE;
    }

    // load media
    SDL_Surface *background = game_loadMedia();
    if (background == NULL) {
        fprintf(stderr, "Error loading media: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    // apply the image
    SDL_BlitSurface(background, NULL, app.windowSurface, NULL);

    // update the surface
    SDL_UpdateWindowSurface(app.window);

    // hack for window to stay up
    SDL_Event e; bool quit = false; while( quit == false ){ while( SDL_PollEvent( &e ) ){ if( e.type == SDL_QUIT ) quit = true; } }

    game_close(&app);
    return 0;
}

// initialize SDL systems and the given app's
// window and window surface. returns true
// if everything goes according to plan and
// false if any step fails
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
    if (app->windowSurface == NULL) {
        fprintf(stderr, "Surface could not be created! SDL_ERROR: %s\n",
            SDL_GetError());
        return false;
    }

    return true;
}

// load a single surface and return a pointer to it
SDL_Surface *game_loadMedia(void)
{
    SDL_Surface *s = SDL_LoadBMP("lena.bmp"); 
    if (s == NULL) {
        return NULL;
    }

    return s;
}

void game_close(struct App *app)
{
    // destroy window
    SDL_DestroyWindow(app->window);
    app->window = NULL;

    // quit SDL systems
    SDL_Quit();
}
