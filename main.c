#include <SDL2/SDL.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

enum KeyPressSurfaces {
    KEY_PRESS_SURFACE_UP,
    KEY_PRESS_SURFACE_DOWN,
    KEY_PRESS_SURFACE_LEFT,
    KEY_PRESS_SURFACE_RIGHT,
    KEY_PRESS_SURFACE_TOTAL
};

struct App {
    SDL_Window *window;
    SDL_Surface *windowSurface;
};

bool game_initializeGame(struct App *app);
SDL_Surface *game_loadMedia(void);
void game_close(struct App *app);
bool game_loadImages(SDL_Surface *arr[]);

int main(void)
{
    struct App app;
    if (!game_initializeGame(&app))
        return EXIT_FAILURE;

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

    // store different background images
    SDL_Surface *images[KEY_PRESS_SURFACE_TOTAL];
    if (!game_loadImages(images))
        return EXIT_FAILURE;

    // quit flag
    bool quit = false;

    // event handler
    SDL_Event e;

    while (!quit) {
        // handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT)
                quit = true;

            SDL_Surface *newImage = NULL;
            int keysym = e.key.keysym.sym;
            switch (keysym) {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                case SDLK_UP:
                    newImage = images[KEY_PRESS_SURFACE_UP];
                    break;
                case SDLK_DOWN:
                    newImage = images[KEY_PRESS_SURFACE_DOWN];
                    break;
                case SDLK_LEFT:
                    newImage = images[KEY_PRESS_SURFACE_RIGHT];
                    break;
                case SDLK_RIGHT:
                    newImage = images[KEY_PRESS_SURFACE_LEFT];
                    break;
            }

            SDL_BlitSurface(newImage, NULL, app.windowSurface, NULL);
            SDL_UpdateWindowSurface(app.window);
        }
    }

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

// initialize array of surface pointers
bool game_loadImages(SDL_Surface *arr[])
{
    arr[KEY_PRESS_SURFACE_UP] = SDL_LoadBMP("dots.bmp");
    if (arr[KEY_PRESS_SURFACE_UP] == NULL) {
        fprintf(stderr, "Error loading image: %s\n", SDL_GetError());
        return false;
    }

    arr[KEY_PRESS_SURFACE_DOWN] = SDL_LoadBMP("snail.bmp");
    if (arr[KEY_PRESS_SURFACE_DOWN] == NULL) {
        fprintf(stderr, "Error loading image: %s\n", SDL_GetError());
        return false;
    }

    arr[KEY_PRESS_SURFACE_LEFT] = SDL_LoadBMP("colors.bmp");
    if (arr[KEY_PRESS_SURFACE_LEFT] == NULL) {
        fprintf(stderr, "Error loading image: %s\n", SDL_GetError());
        return false;
    }

    arr[KEY_PRESS_SURFACE_RIGHT] = SDL_LoadBMP("buck.bmp");
    if (arr[KEY_PRESS_SURFACE_RIGHT] == NULL) {
        fprintf(stderr, "Error loading image: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void game_close(struct App *app)
{
    // destroy window
    SDL_DestroyWindow(app->window);
    app->window = NULL;

    // quit SDL systems
    SDL_Quit();
}
