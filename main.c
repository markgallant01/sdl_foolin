#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const char default_image_path[] = "images/png/blue.png";
const char up_image_path[] = "images/png/bee.png";
const char down_image_path[] = "images/png/boat.png";
const char left_image_path[] = "images/png/clouds.png";
const char right_image_path[] = "images/png/hut.png";

enum KeyPressSurfaces {
    KEY_SURFACE_DEFAULT,
    KEY_SURFACE_UP,
    KEY_SURFACE_DOWN,
    KEY_SURFACE_LEFT,
    KEY_SURFACE_RIGHT,
    KEY_SURFACE_TOTAL
};

struct App {
    SDL_Window *window;
    SDL_Renderer *renderer;
};

bool initialize_game(struct App *app);
void terminate(struct App *app, SDL_Texture *images[]);
bool load_images(SDL_Texture *arr[], struct App *app);
SDL_Texture *load_texture(const char path[], struct App *app);

int main(void)
{
    struct App app;
    if (!initialize_game(&app))
        return EXIT_FAILURE;

    // load media
    SDL_Texture *images[KEY_SURFACE_TOTAL];
    if (!load_images(images, &app))
        return EXIT_FAILURE;

    // clear screen
    SDL_RenderClear(app.renderer);

    // render texture to screen
    SDL_RenderCopy(app.renderer, images[KEY_SURFACE_DEFAULT],
            NULL, NULL);

    // update screen
    SDL_RenderPresent(app.renderer);

    // quit flag
    bool quit = false;

    // event handler
    SDL_Event e;

    while (!quit) {
        // handle events on queue
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT)
                quit = true;

            if (e.type == SDL_KEYDOWN) {
                SDL_Texture *newImage = NULL;
                int keysym = e.key.keysym.sym;
                switch (keysym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_UP:
                        newImage = images[KEY_SURFACE_UP];
                        break;
                    case SDLK_DOWN:
                        newImage = images[KEY_SURFACE_DOWN];
                        break;
                    case SDLK_LEFT:
                        newImage = images[KEY_SURFACE_RIGHT];
                        break;
                    case SDLK_RIGHT:
                        newImage = images[KEY_SURFACE_LEFT];
                        break;
                }

                SDL_RenderClear(app.renderer);
                SDL_RenderCopy(app.renderer, newImage, NULL, NULL);
                SDL_RenderPresent(app.renderer);
            }
        }

    }

    terminate(&app, images);
    return 0;
}

// initialize SDL systems and the given app's
// window and window surface. returns true
// if everything goes according to plan and
// false if any step fails
bool initialize_game(struct App *app)
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

    app->renderer = SDL_CreateRenderer(app->window, -1,
            SDL_RENDERER_ACCELERATED);
    if (app->renderer == NULL) {
        fprintf(stderr, "Renderer could not be created!\n");
        fprintf(stderr, "SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // initialize renderer color
    SDL_SetRenderDrawColor(app->renderer, 0xFF, 0xFF, 0xFF, 0xFF);

    // initialize IMG system
    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    int initted = IMG_Init(flags);
    if ((initted & flags) != flags) {
        fprintf(stderr, "IMG_Init: Failed to initialize\n");
        fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
        return false;
    }

    return true;
}

// load an image from a given filepath optimized for the 
// passed in App
SDL_Texture *load_texture(const char path[], struct App *app)
{
    SDL_Surface *rawImage = IMG_Load(path);
    if (rawImage == NULL) {
        fprintf(stderr, "Error loading image: %s\n \
                Error: %s\n", path, IMG_GetError());
        return NULL;
    }

    SDL_Texture *newTexture =
        SDL_CreateTextureFromSurface(app->renderer, rawImage);
    if (newTexture == NULL) {
        fprintf(stderr, "Error creating texture\n");
        fprintf(stderr, "SDL_Error: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_FreeSurface(rawImage);

    return newTexture;
}

// initialize array of surface pointers
bool load_images(SDL_Texture *arr[], struct App *app)
{
    arr[KEY_SURFACE_DEFAULT] = load_texture(default_image_path, app);
    if (arr[KEY_SURFACE_DEFAULT] == NULL) {
        fprintf(stderr, "Error loading default image: %s\n",
                SDL_GetError());
        return false;
    }

    arr[KEY_SURFACE_UP] = load_texture(up_image_path, app);
    if (arr[KEY_SURFACE_UP] == NULL) {
        fprintf(stderr, "Error loading up image: %s\n", 
                SDL_GetError());
        return false;
    }

    arr[KEY_SURFACE_DOWN] = load_texture(down_image_path, app);
    if (arr[KEY_SURFACE_DOWN] == NULL) {
        fprintf(stderr, "Error loading down image: %s\n", 
                SDL_GetError());
        return false;
    }

    arr[KEY_SURFACE_LEFT] = load_texture(left_image_path, app);
    if (arr[KEY_SURFACE_LEFT] == NULL) {
        fprintf(stderr, "Error loading left image: %s\n",
                SDL_GetError());
        return false;
    }

    arr[KEY_SURFACE_RIGHT] = load_texture(right_image_path, app);
    if (arr[KEY_SURFACE_RIGHT] == NULL) {
        fprintf(stderr, "Error loading right image: %s\n",
                SDL_GetError());
        return false;
    }

    return true;
}

void terminate(struct App *app, SDL_Texture *images[])
{
    // free loaded images
    for (int i = 0; i < KEY_SURFACE_TOTAL; i++) {
        SDL_DestroyTexture(images[i]);
    }

    // destroy window & renderer
    SDL_DestroyRenderer(app->renderer);
    app->renderer = NULL;
    SDL_DestroyWindow(app->window);
    app->window = NULL;

    // quit SDL systems
    IMG_Quit();
    SDL_Quit();
}
