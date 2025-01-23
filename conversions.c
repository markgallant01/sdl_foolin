#include "conversions.h"

float pixelsToMeters(int pixels)
{
    return (float)pixels / CONV_FACTOR;
}

int metersToPixels(float meters)
{
    return (int)(meters * CONV_FACTOR);
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
