#include "engine/arcball.h"

void arcball_init(arcball_t* ab)
{
    glm_vec3_copy((vec3) { 0.0f, 0.0f, 5.0f }, ab->eye);
    glm_vec3_copy((vec3) { 0.0f, 0.0f, 0.0f }, ab->look_at);
    glm_vec3_copy((vec3) { 0.0f, 1.0f, 0.0f }, ab->up);
    ab->radius   = 1.0f;
    ab->zoom     = 1.0f;
    ab->distance = 5.0f;

    glm_quat_identity(ab->rotation);

    arcball_update_view_matrix(ab);
}

void arcball_update_view_matrix(arcball_t* ab)
{
    vec3 offset = { 0.0f, 0.0f, ab->distance };

    // Rotate the offset vector using the quaternion
    glm_quat_rotatev(ab->rotation, offset, offset);

    // Compute new camera position
    glm_vec3_add(ab->look_at, offset, ab->eye);

    // Generate view matrix
    glm_lookat(ab->eye, ab->look_at, ab->up, ab->view);
}

void arcball_rotate(arcball_t* ab, float angle, vec3 axis)
{
    // Normalize the rotation axis
    glm_vec3_normalize(axis);

    // Create a quaternion representing the rotation
    vec4 q;
    glm_quat(q, glm_rad(angle), axis[0], axis[1], axis[2]); // glm_quat(angle, axis, dest)

    // Apply the rotation by multiplying the quaternions
    vec4 temp;
    glm_quat_mul(ab->rotation, q, temp); // temp = ab->rotation * q
    glm_quat_copy(temp, ab->rotation);   // Store result back in ab->rotation

    // Normalize quaternion to prevent numerical drift
    glm_quat_normalize(ab->rotation);

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