#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <box2d/box2d.h>

#include <stdio.h>

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;

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

struct PixelCoords { 
    int x;
    int y;
};

struct MeterCoords {
    float x;
    float y;
};

const struct PixelCoords pxOrigin = {
    .x = SCREEN_WIDTH / 2,
    .y = SCREEN_HEIGHT / 2
};

bool initialize_game(struct App *app);
void terminate(struct App *app);
bool load_images(SDL_Texture *arr[], struct App *app);
SDL_Texture *load_texture(const char path[], struct App *app);
float pixelsToMeters(int pixels);
int metersToPixels(float meters);
struct pxVect coordConvert(struct pxVect oldCoords);
struct pxVect cornerConvert(int x, int y, int width, int height);
void render_grid(struct App *app);
struct PixelCoords meterCoordsToPx(struct MeterCoords mCoords);
void createLineAtMeters(struct App *app, struct MeterCoords mCoords);
void createBoxAtMeters(struct App *app, struct MeterCoords mCoords, int width,
        int height);

int main(void)
{
    struct App app;
    if (!initialize_game(&app))
        return EXIT_FAILURE;

    // create physics worlds
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = (b2Vec2){0.0f, -10.0f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // create ground body
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = (b2Vec2){0.0f, -7.0f};
    b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

    // create ground polygon centered on ground body
    b2Polygon groundBox  = b2MakeBox(4.0f, 1.0f);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    // create dynamic drop box
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = (b2Vec2){0.0f, 7.0f};
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    // drop box shape
    b2Polygon dynamicBox = b2MakeBox(0.5f, 0.5f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.3f;
    b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

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
            }
        }

        // clear screen
        SDL_SetRenderDrawColor(app.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(app.renderer);

        render_grid(&app);

        // advance physics world
        b2World_Step(worldId, timeStep, subStepCount);

        struct MeterCoords groundBox = {.x = 0.0f, .y = -7.0f};
        createBoxAtMeters(&app, groundBox, 400, 100);

        // get position of cube
        b2Vec2 position = b2Body_GetPosition(bodyId);

        struct MeterCoords dropBox = {.x = position.x, .y = position.y};
        createBoxAtMeters(&app, dropBox, 50, 50);

        SDL_RenderPresent(app.renderer);
    }

    terminate(&app);
    return 0;
}

// pixel coords can be converted from meters by relating them
// to the calculated pixel origin
struct PixelCoords meterCoordsToPx(struct MeterCoords mCoords)
{
    printf("initial coords:\n");
    printf("meterX: %f meterY: %f\n", mCoords.x, mCoords.y);
    // convert meter coords to pixels
    int pixelX = metersToPixels(mCoords.x);
    int pixelY = metersToPixels(mCoords.y);
    printf("Converted raw coords:\n");
    printf("raw PX-x: %d raw PX-y: %d\n", pixelX, pixelY);

    // convert X by relating to the origin. Positive values
    // get added to the origin and negative values subtracted
    pixelX = pxOrigin.x + pixelX;
    pixelY = pxOrigin.y - pixelY;
    printf("final coords:\n");
    printf("final x: %d final y: %d\n", pixelX, pixelY);

    struct PixelCoords pxCoords = {.x = pixelX, .y = pixelY};
    return pxCoords;
}

struct MeterCoords pixelCoordsToMeters(struct PixelCoords pxCoords)
{
    struct MeterCoords mCoords;
    mCoords.x = pixelsToMeters(pxCoords.x);
    mCoords.y = pixelsToMeters(pxCoords.y);
    return mCoords;
}

void createBoxAtMeters(struct App *app, struct MeterCoords mCoords, int width,
        int height)
{
    SDL_SetRenderDrawColor(app->renderer, 0xFF, 0x00, 0x00, 0xFF);
    struct PixelCoords pxCoords = meterCoordsToPx(mCoords);
    // convert coords from center-origin to corner-origin
    pxCoords.x -= width / 2;
    pxCoords.y -= height / 2;
    SDL_Rect fillRect = {.x = pxCoords.x, .y = pxCoords.y, width, height};
    SDL_RenderFillRect(app->renderer, &fillRect);
}

void createLineAtMeters(struct App *app, struct MeterCoords mCoords)
{
    SDL_SetRenderDrawColor(app->renderer, 0xFF, 0x00, 0x00, 0xFF);
    struct PixelCoords pxCoords = meterCoordsToPx(mCoords);
    SDL_RenderDrawLine(app->renderer, pxCoords.x - 20, pxCoords.y,
            pxCoords.x + 20, pxCoords.y);
}

void render_grid(struct App *app) {
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
