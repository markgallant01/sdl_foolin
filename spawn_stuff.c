#include "spawn_stuff.h"
#include "conversions.h"

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
