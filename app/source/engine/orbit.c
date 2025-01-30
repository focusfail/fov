#include "engine/orbit.h"

void orbit_update(orbit_cam_t* orbit)
{
    orbit->pitch = glm_clamp(orbit->pitch, -GLM_PI_2 + 0.01f, GLM_PI_2 - 0.01f);

    orbit->position[0] = orbit->target[0] + orbit->radius * cosf(orbit->pitch) * sinf(orbit->yaw);
    orbit->position[1] = orbit->target[1] + orbit->radius * sinf(orbit->pitch);
    orbit->position[2] = orbit->target[2] + orbit->radius * cosf(orbit->pitch) * cosf(orbit->yaw);

    glm_lookat(orbit->position, orbit->target, GLM_YUP, orbit->view);
}

void orbit_init(orbit_cam_t* orbit)
{
    orbit->pitch = 0.0f;
    orbit->yaw   = 0.0f;
    glm_vec3_copy(orbit->position, (vec3) { 1.0f, 1.0f, 1.0f });
    glm_vec3_copy(orbit->target, (vec3) { 0.0f, 0.0f, 0.0f });
    orbit_update(orbit);
}