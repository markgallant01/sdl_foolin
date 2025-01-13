#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <box2d/box2d.h>

#include <stdio.h>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// pixel - meter conversion factor
const float CONV_FACTOR = 100.0f;

const char default_image_path[] = "images/png/blue.png";
const char up_image_path[] = "images/png/bee.png";
const char down_image_path[] = "images/png/boat.png";
const char left_image_path[] = "images/png/clouds.png";
const char right_image_path[] = "images/png/hut.png";

enum KeyPressTextures {
    KEY_TEXTURE_DEFAULT,
    KEY_TEXTURE_UP,
    KEY_TEXTURE_DOWN,
    KEY_TEXTURE_LEFT,
    KEY_TEXTURE_RIGHT,
    KEY_TEXTURE_TOTAL
};

struct App {
    SDL_Window *window;
    SDL_Renderer *renderer;
};

bool initialize_game(struct App *app);
void terminate(struct App *app, SDL_Texture *images[]);
bool load_images(SDL_Texture *arr[], struct App *app);
SDL_Texture *load_texture(const char path[], struct App *app);
float pixelsToMeters(int pixels);
int metersToPixels(float meters);

int main(void)
{
    struct App app;
    if (!initialize_game(&app))
        return EXIT_FAILURE;

    // set up physics world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, -10.0f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // ground physics body
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = (b2Vec2){0.0f, -10.0f};
    b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

    // ground body polygon
    b2Polygon groundBox = b2MakeBox(50.0f, 10.0f);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    // clear screen
    SDL_SetRenderDrawColor(app.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(app.renderer);

    // red square dynamic body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){0.0f, 4.0f};
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    // red square body shape
    b2Polygon dynamicBox = b2MakeBox(1.0f, 1.0f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

    SDL_RenderPresent(app.renderer);

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
            }

        }

        // physics tick
        b2World_Step(worldId, timeStep, subStepCount);

        // clear screen
        SDL_SetRenderDrawColor(app.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(app.renderer);

        // update red square position
        b2Vec2 position = b2Body_GetPosition(bodyId);
        b2Rot rotation = b2Body_GetRotation(bodyId);
        printf("%4.2f %4.2f %4.2f", position.x, position.y,
                b2Rot_GetAngle(rotation));
        break;
    }

    return 0;
}

float pixelsToMeters(int pixels)
{
    return (float)pixels / CONV_FACTOR;
}

int metersToPixels(float meters)
{
    return (int)(meters * CONV_FACTOR);
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
    SDL_Texture *new_texture = IMG_LoadTexture(app->renderer, path);
    if (new_texture == NULL) {
        fprintf(stderr, "Error creating texture\n");
        fprintf(stderr, "SDL_Error: %s\n", SDL_GetError());
        return NULL;
    }

    return new_texture;
}

// initialize array of surface pointers
bool load_images(SDL_Texture *arr[], struct App *app)
{
    arr[KEY_TEXTURE_DEFAULT] = load_texture(default_image_path, app);
    if (arr[KEY_TEXTURE_DEFAULT] == NULL) {
        fprintf(stderr, "Error loading default image: %s\n",
                SDL_GetError());
        return false;
    }

    arr[KEY_TEXTURE_UP] = load_texture(up_image_path, app);
    if (arr[KEY_TEXTURE_UP] == NULL) {
        fprintf(stderr, "Error loading up image: %s\n", 
                SDL_GetError());
        return false;
    }

    arr[KEY_TEXTURE_DOWN] = load_texture(down_image_path, app);
    if (arr[KEY_TEXTURE_DOWN] == NULL) {
        fprintf(stderr, "Error loading down image: %s\n", 
                SDL_GetError());
        return false;
    }

    arr[KEY_TEXTURE_LEFT] = load_texture(left_image_path, app);
    if (arr[KEY_TEXTURE_LEFT] == NULL) {
        fprintf(stderr, "Error loading left image: %s\n",
                SDL_GetError());
        return false;
    }

    arr[KEY_TEXTURE_RIGHT] = load_texture(right_image_path, app);
    if (arr[KEY_TEXTURE_RIGHT] == NULL) {
        fprintf(stderr, "Error loading right image: %s\n",
                SDL_GetError());
        return false;
    }

    return true;
}

void terminate(struct App *app, SDL_Texture *images[])
{
    // free loaded images
    for (int i = 0; i < KEY_TEXTURE_TOTAL; i++) {
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
