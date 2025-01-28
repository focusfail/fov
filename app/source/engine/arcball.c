#include "engine/arcball.h"

void arcball_init(arcball_t* ab)
{
    glm_vec3_copy((vec3) { 0.0f, 0.0f, 5.0f }, ab->eye);
    glm_vec3_copy((vec3) { 0.0f, 0.0f, 0.0f }, ab->look_at);
    glm_vec3_copy((vec3) { 0.0f, 1.0f, 0.0f }, ab->up);
    ab->radius   = 1.0f;
    ab->zoom     = 1.0f;
    ab->distance = 5.0f;
    ab->pitch    = 0.0f;
    ab->yaw      = 0.0f;
    glm_mat4_identity(ab->rotation);
    arcball_update_view_matrix(ab);
}

void arcball_update_view_matrix(arcball_t* ab)
{
    vec3 offset = { 0.0f, 0.0f, ab->distance };
    glm_mat4_mulv3(ab->rotation, offset, 1.0f, offset);
    glm_vec3_add(ab->look_at, offset, ab->eye);
    glm_lookat(ab->eye, ab->look_at, ab->up, ab->view);
}

void arcball_rotate(arcball_t* ab, float angle, vec3 axis)
{
    // Limit angle
    const float MAX_ANGLE = 0.1f;
    angle                 = glm_clamp(angle, -MAX_ANGLE, MAX_ANGLE);

    // Disable roll
    axis[2] = 0.0f;

    // Handle pitch (X-axis)
    if (axis[0] != 0.0f) {
        const float PITCH_LIMIT = 85.0f;
        float       new_pitch   = ab->pitch + glm_deg(angle * axis[0]);
        if (new_pitch > PITCH_LIMIT) new_pitch = PITCH_LIMIT;
        if (new_pitch < -PITCH_LIMIT) new_pitch = -PITCH_LIMIT;
        ab->pitch = new_pitch;
    }

    // Handle yaw (Y-axis)
    if (axis[1] != 0.0f) {
        ab->yaw += glm_deg(angle * axis[1]);
        // Normalize yaw to prevent extreme twisting
        if (ab->yaw < -180.0f) ab->yaw += 360.0f;
        if (ab->yaw > 180.0f) ab->yaw -= 360.0f;
    }

    // Rebuild rotation from pitch/yaw
    glm_mat4_identity(ab->rotation);
    glm_rotate_x(ab->rotation, glm_rad(ab->pitch), ab->rotation);
    glm_rotate_y(ab->rotation, glm_rad(ab->yaw), ab->rotation);

    arcball_update_view_matrix(ab);
}
void arcball_zoom(arcball_t* ab, float zoom_factor)
{
    // Add zoom limits
    const float MIN_DISTANCE = 0.1f;
    const float MAX_DISTANCE = 20.0f;

    ab->distance *= zoom_factor;
    ab->distance = glm_clamp(ab->distance, MIN_DISTANCE, MAX_DISTANCE);

    arcball_update_view_matrix(ab);
}