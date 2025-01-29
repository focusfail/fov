#ifndef __ORBIT_H__
#define __ORBIT_H__

#include "cglm/cglm.h"

typedef struct {
    float radius;
    float yaw;
    float pitch;
    vec3  position;
    vec3  target;
    mat4  view;

} orbit_cam_t;

void orbit_init(orbit_cam_t* orbit);
void orbit_update(orbit_cam_t* orbit);

#endif // __ORBIT_H__