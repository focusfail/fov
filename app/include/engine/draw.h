#ifndef __ENGINE_DRAW_H__
#define __ENGINE_DRAW_H__

#include "cglm/cglm.h"

void draw_init(int screen_width, int screen_height);
void draw_flush();
void draw_cleanup();

void draw_rectangle(float x, float y, float width, float height, vec3 color);

#endif // __ENGINE_DRAW_H__