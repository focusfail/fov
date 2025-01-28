#ifndef ARCBALL_H
#define ARCBALL_H

#include <cglm/cglm.h>

typedef struct {
    vec3  eye;
    vec3  look_at;
    vec3  up;
    mat4  view;
    float radius;
    float zoom;
    float pitch;
    float yaw;
    float distance;
    mat4  rotation;
} arcball_t;

void arcball_init(arcball_t* ab);
void arcball_set_view(arcball_t* ab, vec3 eye, vec3 look_at, vec3 up);
void arcball_update_view_matrix(arcball_t* ab);
void arcball_rotate(arcball_t* ab, float angle, vec3 axis);
void arcball_zoom(arcball_t* ab, float zoom_factor);

#endif // ARCBALL_H