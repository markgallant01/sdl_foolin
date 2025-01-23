#ifndef SPAWN_STUFF_H
#define SPAWN_STUFF_H

#include "defs.h"

#include <box2d/box2d.h>

b2BodyId createBox(float mX, float mY, b2WorldId worldId);
b2BodyId createGroundBlock(float mX, float mY, b2WorldId worldId);
void createBoxAtMeters(struct App *app, struct MeterCoords mCoords, int width,
        int height);
void createLineAtMeters(struct App *app, struct MeterCoords mCoords);


#endif
