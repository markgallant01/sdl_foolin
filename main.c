#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <box2d/box2d.h>

#include <stdio.h>

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;

// pixel - meter conversion factor
const float CONV_FACTOR = 70.0f;

struct App {
    SDL_Window *window;
    SDL_Renderer *renderer;
};

struct Node {
    b2BodyId block;
    struct Node *next;
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

enum Textures {
    GROUND_TEXTURE,
    BOX_TEXTURE,
    TEXTURES_TOTAL,
};

const char *ground_texture_path = "assets/platformer_art/Tiles/grassCenter.png";
const char *box_texture_path = "assets/platformer_art/Tiles/box.png";

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
b2BodyId createGroundBlock(float pxX, float pxY, b2WorldId worldId);
b2BodyId createBox(float pxX, float pxY, b2WorldId worldId);
void render_ground_block(b2Vec2 position, struct App *app,
        SDL_Texture *textures[]);
void render_box_block(b2Vec2 position, struct App *app,
        SDL_Texture *textures[]);
void render_dynamic_box(b2Vec2 position, struct App *app,
        SDL_Texture *textures[]);

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

// render dynamic box
void render_dynamic_box(b2Vec2 position, struct App *app,
        SDL_Texture *textures[])
{
    struct MeterCoords mCoords;
    mCoords.x = position.x;
    mCoords.y = position.y;
    struct PixelCoords pxCoords = meterCoordsToPx(mCoords);
    pxCoords.x -= 70 / 2;
    pxCoords.y -= 70 / 2;
    SDL_Rect renderQuad = {.x = pxCoords.x, .y = pxCoords.y, .w = 70, .h = 70};
    SDL_RenderCopy(app->renderer, textures[BOX_TEXTURE], NULL, &renderQuad);
}

// render ground blocks with ground texture
void render_ground_block(b2Vec2 position, struct App *app,
        SDL_Texture *textures[])
{
    // set rendering space and render to screen
    struct MeterCoords mCoords;
    mCoords.x = position.x;
    mCoords.y = position.y;
    struct PixelCoords pxCoords = meterCoordsToPx(mCoords);
    pxCoords.x -= 70 / 2;
    pxCoords.y -= 70 / 2;
    SDL_Rect renderQuad = {.x = pxCoords.x, .y = pxCoords.y, .w = 70, .h = 70};
    SDL_RenderCopy(app->renderer, textures[GROUND_TEXTURE], NULL, &renderQuad);
}

void render_box_block(b2Vec2 position, struct App *app, SDL_Texture *textures[])
{
    struct MeterCoords mCoords;
    mCoords.x = position.x;
    mCoords.y = position.y;
    struct PixelCoords pxCoords = meterCoordsToPx(mCoords);
    SDL_Rect renderQuad = {.x = pxCoords.x, .y = pxCoords.y, .w = 70, .h = 70};
    SDL_RenderCopy(app->renderer, textures[BOX_TEXTURE], NULL, &renderQuad);
}

// create dynamic box at the given (x, y) meter coordinates
b2BodyId createBox(float mX, float mY, b2WorldId worldId)
{
    // convert pixel coords to meters
    struct b2Vec2 m_coords;
    m_coords.x = mX;
    m_coords.y = mY;

    // create dynamic drop box
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = m_coords;
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    // drop box shape
    b2Polygon dynamicBox = b2MakeBox(0.5f, 0.5f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    shapeDef.friction = 0.3f;
    shapeDef.restitution = 0.3f;
    b2CreatePolygonShape(bodyId, &shapeDef, &dynamicBox);

    return bodyId;
}

// create a ground block at the given (x,y) meter coordinates
b2BodyId createGroundBlock(float mX, float mY, b2WorldId worldId)
{
    const int GROUND_BLOCK_PX_W = 70;
    const int GROUND_BLOCK_PX_H = 70;

    const float GROUND_BLOCK_M_W = pixelsToMeters(GROUND_BLOCK_PX_W);
    const float GROUND_BLOCK_M_H = pixelsToMeters(GROUND_BLOCK_PX_H);

    struct b2Vec2 m_coords;
    m_coords.x = mX;
    m_coords.y = mY;

    // create ground body
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = m_coords;
    b2BodyId groundId = b2CreateBody(worldId, &groundBodyDef);

    // create ground polygon centered on ground body
    b2Polygon groundBox  = b2MakeBox(GROUND_BLOCK_M_W / 2,
            GROUND_BLOCK_M_H / 2);
    b2ShapeDef groundShapeDef = b2DefaultShapeDef();
    b2CreatePolygonShape(groundId, &groundShapeDef, &groundBox);

    return groundId;
}

// pixel coords can be converted from meters by relating them
// to the calculated pixel origin
struct PixelCoords meterCoordsToPx(struct MeterCoords mCoords)
{
    // convert meter coords to pixels
    int pixelX = metersToPixels(mCoords.x);
    int pixelY = metersToPixels(mCoords.y);

    // convert X by relating to the origin. Positive values
    // get added to the origin and negative values subtracted
    pixelX = pxOrigin.x + pixelX;
    pixelY = pxOrigin.y - pixelY;

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

    // starting at origin, draw lines going up
    int mid_y = SCREEN_HEIGHT / 2;
    int mid_x = SCREEN_WIDTH / 2;
    int step = (int)CONV_FACTOR;    // step = 1 meter
    for (int i = mid_y - step; i > 0; i -= step) {
        SDL_RenderDrawLine(app->renderer, mid_x -10, i, mid_x + 10, i);
    }

    // starting at origin, draw lines going down
    step = (int)CONV_FACTOR;
    for (int i = mid_y + step; i < SCREEN_HEIGHT; i += step) {
        SDL_RenderDrawLine(app->renderer, mid_x - 10, i, mid_x + 10, i);
    }

    // starting at origin, draw lines going right
    step = (int)CONV_FACTOR;
    for (int i = mid_x + step; i < SCREEN_WIDTH; i += step) {
        SDL_RenderDrawLine(app->renderer, i, mid_y - 10, i, mid_y + 10);
    }

    // starting at origin, draw lines going left
    step = (int)CONV_FACTOR;
    for (int i = mid_x - step; i > 0; i -= step) {
        SDL_RenderDrawLine(app->renderer, i, mid_y - 10, i, mid_y + 10);
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
