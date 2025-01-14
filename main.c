#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <box2d/box2d.h>

#include <stdio.h>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

// pixel - meter conversion factor
const float CONV_FACTOR = 50.0f;

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

struct pxVect {
    int x;
    int y;
};

bool initialize_game(struct App *app);
void terminate(struct App *app, SDL_Texture *images[]);
bool load_images(SDL_Texture *arr[], struct App *app);
SDL_Texture *load_texture(const char path[], struct App *app);
float pixelsToMeters(int pixels);
int metersToPixels(float meters);
struct pxVect coordConvert(struct pxVect oldCoords);
struct pxVect cornerConvert(int x, int y, int width, int height);
void render_grid(struct App *app);

int main(void)
{
    struct App app;
    if (!initialize_game(&app))
        return EXIT_FAILURE;

    // set up physics world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, -1.0f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // ground physics body
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    // position origin 200 pixels down
    float yPosition = pixelsToMeters(200);
    groundBodyDef.position = (b2Vec2){0.0f, -yPosition};
    b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

    // ground body polygon
    // 200px wide, 100 tall
    float mWidth = pixelsToMeters(200);
    float mHeight = pixelsToMeters(100);
    // halve, b2MakeBox takes half-lengths
    mWidth /= 2;
    mHeight /= 2;
    b2Polygon groundBox = b2MakeBox(mWidth, mHeight);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    groundShapeDef.restitution = 0.5f;
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    // clear screen
    SDL_SetRenderDrawColor(app.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(app.renderer);

    // red square dynamic body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    // position 200 meters up
    yPosition = pixelsToMeters(200);
    bodyDef.position = (b2Vec2){0.0f, yPosition};
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    // red square body shape
    // 20px by 20px box
    mWidth = pixelsToMeters(20);
    mHeight = pixelsToMeters(20);
    // halve for half-lengths
    mWidth /= 2;
    mHeight /= 2;
    b2Polygon dynamicBox = b2MakeBox(mWidth, mHeight);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

    // render ground
    b2Vec2 groundPos = b2Body_GetPosition(groundId);
    int pxX = metersToPixels(groundPos.x);
    int pxY = metersToPixels(groundPos.y);
    struct pxVect groundPxCoords = cornerConvert(pxX, pxY, 200, 100);
    groundPxCoords = coordConvert(groundPxCoords);
    SDL_Rect groundRect = {.x = groundPxCoords.x, .y = groundPxCoords.y,
        .w = 200, .h = 100};
    SDL_SetRenderDrawColor(app.renderer, 0x00, 0xFF, 0x00, 0xFF);
    SDL_RenderFillRect(app.renderer, &groundRect);

    // render red cube
    b2Vec2 cubePos = b2Body_GetPosition(bodyId);
    pxX = metersToPixels(cubePos.x);
    pxY = metersToPixels(cubePos.y);
    struct pxVect cubePxCoords = cornerConvert(pxX, pxY, 20, 20);
    cubePxCoords = coordConvert(cubePxCoords);
    SDL_Rect cubeRect = {.x = cubePxCoords.x, .y = cubePxCoords.y,
        .w = 20, .h = 20};
    SDL_SetRenderDrawColor(app.renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(app.renderer, &cubeRect);

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

        render_grid(&app);

        // render ground
        b2Vec2 groundPos = b2Body_GetPosition(groundId);
        int pxX = metersToPixels(groundPos.x);
        int pxY = metersToPixels(groundPos.y);
        struct pxVect groundPxCoords = cornerConvert(pxX, pxY, 200, 100);
        groundPxCoords = coordConvert(groundPxCoords);
        SDL_Rect groundRect = {.x = groundPxCoords.x, .y = groundPxCoords.y,
            .w = 200, .h = 100};
        SDL_SetRenderDrawColor(app.renderer, 0x00, 0xFF, 0x00, 0xFF);
        SDL_RenderFillRect(app.renderer, &groundRect);

        // render red cube
        b2Vec2 cubePos = b2Body_GetPosition(bodyId);
        pxX = metersToPixels(cubePos.x);
        pxY = metersToPixels(cubePos.y);
        struct pxVect cubePxCoords = cornerConvert(pxX, pxY, 20, 20);
        cubePxCoords = coordConvert(cubePxCoords);
        SDL_Rect cubeRect = {.x = cubePxCoords.x, .y = cubePxCoords.y,
            .w = 20, .h = 20};
        SDL_SetRenderDrawColor(app.renderer, 0xFF, 0x00, 0x00, 0xFF);
        SDL_RenderFillRect(app.renderer, &cubeRect);

        SDL_RenderPresent(app.renderer);
    }

    return 0;
}

void render_grid(struct App *app)
{
    SDL_SetRenderDrawColor(app->renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderDrawLine(app->renderer, SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2,
            SCREEN_HEIGHT);
    SDL_RenderDrawLine(app->renderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH,
            SCREEN_HEIGHT / 2);

    int vStep = SCREEN_HEIGHT / 20;
    int mid = SCREEN_WIDTH / 2;
    for (int i = vStep; i < SCREEN_HEIGHT; i += vStep) {
        SDL_RenderDrawLine(app->renderer, mid - 10, i, mid + 10, i);
    }

    vStep = SCREEN_WIDTH / 20;
    mid = SCREEN_HEIGHT / 2;
    for (int i = vStep; i < SCREEN_WIDTH; i += vStep) {
        SDL_RenderDrawLine(app->renderer, i, mid + 10, i, mid - 10);
    }
}

float pixelsToMeters(int pixels)
{
    return (float)pixels / CONV_FACTOR;
}

int metersToPixels(float meters)
{
    return (int)(meters * CONV_FACTOR);
}

struct pxVect cornerConvert(int centerX, int centerY, int width, int height)
{
    struct pxVect cornerCoords;
    cornerCoords.x = (int)((float)centerX - (0.5f * (float)width));
    cornerCoords.y = (int)((float)centerY - (0.5f * (float)height));

    return cornerCoords;
}

struct pxVect coordConvert(struct pxVect oldCoords)
{
    // screen center origin
    int screenOriginX = SCREEN_WIDTH / 2;
    int screenOriginY = SCREEN_HEIGHT / 2;

    oldCoords.x = (int)((float)screenOriginX + (float)oldCoords.x);
    oldCoords.y = (int)((float)screenOriginY - (float)oldCoords.y);

    return oldCoords;
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
