#ifndef __GRID_H__
#define __GRID_H__

#include "cglm/cglm.h"

typedef struct {
    unsigned int vao;
    unsigned int vbo;
    unsigned int cbo;
    unsigned int program;
    int          line_width;
    int          line_count;
    int          size;
} grid_t;

grid_t grid_create();
void   grid_build(grid_t* grid, vec3 center, int size, int line_width, float spacing);
void   grid_destroy(grid_t* grid);
void   grid_render(const grid_t* grid, mat4 projection, mat4 view);

#endif // __GRID_H__