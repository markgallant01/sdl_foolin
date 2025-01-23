#ifndef RENDER_H
#define RENDER_H

#include "defs.h"

#include <box2d/box2d.h>
#include <SDL2/SDL.h>

void render_dynamic_box(b2Vec2 position, struct App *app,
        SDL_Texture *textures[]);
void render_ground_block(b2Vec2 position, struct App *app,
        SDL_Texture *textures[]);
void render_grid(struct App *app);


#endif
