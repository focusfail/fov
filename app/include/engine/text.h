#ifndef __ENGINE_TEXT_H__
#define __ENGINE_TEXT_H__

#include "cglm/cglm.h"

void text_init(const char* font_path, int window_w, int window_h);
void text_draw(const char* text, float x, float y, float scale, vec3 color);
void text_cleanup(void);
void text_update(int width, int height);

#endif // __ENGINE_TEXT_H__