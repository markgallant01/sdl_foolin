#include "defs.h"
#include "spawn_stuff.h"
#include "render.h"

#include <box2d/box2d.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>

#include <stdio.h>

bool initialize_game(struct App *app);
void terminate(struct App *app);
bool load_images(SDL_Texture *arr[], struct App *app);
SDL_Texture *load_texture(const char path[], struct App *app);

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;

// pixel - meter conversion factor
const float CONV_FACTOR = 70.0f;

const struct PixelCoords pxOrigin = {
    .x = SCREEN_WIDTH / 2,
    .y = SCREEN_HEIGHT / 2
};

const char *ground_texture_path = "assets/platformer_art/Tiles/grassCenter.png";
const char *box_texture_path = "assets/platformer_art/Tiles/box.png";

int main(void)
{
    struct App app;
    if (!initialize_game(&app))
        return EXIT_FAILURE;

    SDL_Texture *textures[TEXTURES_TOTAL];
    textures[GROUND_TEXTURE] = IMG_LoadTexture(app.renderer,
            ground_texture_path);
    textures[BOX_TEXTURE] = IMG_LoadTexture(app.renderer, box_texture_path);

    if ((textures[0] == NULL) || textures[1] == NULL) {
        printf("PROBLEM\n");
        return EXIT_FAILURE;
    }

    // create physics worlds
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, -10.0f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // world LL
    struct Node world;
    world.next = NULL;
    world.block = createGroundBlock(0.0f, -5.0f, worldId);

    struct Node *secondNode = malloc(sizeof(struct Node));
    secondNode->next = NULL;
    secondNode->block = createGroundBlock(-1.0f, -5.0f, worldId);
    world.next = secondNode;

    struct Node *thirdNode = malloc(sizeof(struct Node));
    thirdNode->next = NULL;
    thirdNode->block = createGroundBlock(1.0f, -5.0f, worldId);
    secondNode->next = thirdNode;

    // dynamic object LL
    struct Node objects;
    objects.next = NULL;
    objects.block = createBox(0.0f, 5.0f, worldId);

    // world setup
    float timeStep = 1.0f / 60.0f;
    int subStepCount = 4;

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
                int keysym = e.key.keysym.sym;
                if (keysym == SDLK_ESCAPE)
                    quit = true;
                if (keysym == SDLK_SPACE)
                    b2Body_ApplyForceToCenter(objects.block,
                            (b2Vec2){0.0f, 500.0f}, true);
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                    printf("xClick: %d yClick: %d\n", e.button.x, e.button.y);
            }
        }

        // clear screen
        SDL_SetRenderDrawColor(app.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(app.renderer);

        render_grid(&app);

        // iterate world LL and render blocks
        struct Node *currentNode = &world;
        while (currentNode != NULL) {
            // get ground block position
            b2Vec2 position = b2Body_GetPosition(currentNode->block);
            render_ground_block(position, &app, textures);
            currentNode = currentNode->next;
        }

        // iterate box LL and render blocks
        currentNode = &objects;
        while (currentNode != NULL) {
            // get dynamic box position
            b2Vec2 position = b2Body_GetPosition(currentNode->block);
            render_dynamic_box(position, &app, textures);
            currentNode = currentNode->next;
        }

        // advance physics world
        b2World_Step(worldId, timeStep, subStepCount);

        SDL_RenderPresent(app.renderer);
    }

    terminate(&app);
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
    app->window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

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

void terminate(struct App *app)
{
    // destroy window & renderer
    SDL_DestroyRenderer(app->renderer);
    app->renderer = NULL;
    SDL_DestroyWindow(app->window);
    app->window = NULL;

    // quit SDL systems
    IMG_Quit();
    SDL_Quit();
}
