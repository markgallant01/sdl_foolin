#include "defs.h"
#include "conversions.h"

#include <box2d/box2d.h>
#include <SDL2/SDL.h>

void render_grid(struct App *app)
{
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
