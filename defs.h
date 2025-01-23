#ifndef DEFS_H
#define DEFS_H

#include <box2d/box2d.h>
#include <SDL2/SDL.h>

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

enum Textures {
    GROUND_TEXTURE,
    BOX_TEXTURE,
    TEXTURES_TOTAL,
};

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

// pixel - meter conversion factor
extern const float CONV_FACTOR;

extern const struct PixelCoords pxOrigin;

extern const char *ground_texture_path;
extern const char *box_texture_path;

#endif
