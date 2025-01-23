#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include "defs.h"

float pixelsToMeters(int pixels);
int metersToPixels(float meters);
struct PixelCoords meterCoordsToPx(struct MeterCoords mCoords);
struct MeterCoords pixelCoordsToMeters(struct PixelCoords pxCoords);

#endif
