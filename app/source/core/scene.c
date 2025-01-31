#include "core/scene.h"

#include "parsers/obj.h"

#include "glad/glad.h"
#include "log.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"

#include "nuklear_glfw_gl4.h"

#include <string.h>

void _scale_model_size(vec3 min, vec3 max, vec3 scaled)
{
    vec3 center, extent, normalized;

    glm_vec3_add(max, min, center);
    glm_vec3_scale(center, 0.5f, center);

    glm_vec3_sub(max, min, extent);
    glm_vec3_scale(extent, 0.5f, extent);

    float maxExtent = fmaxf(fmaxf(extent[0], extent[1]), extent[2]);

    glm_vec3_sub(min, center, normalized);
    glm_vec3_scale(normalized, 1.0f / maxExtent, normalized);
    glm_vec3_scale(normalized, 1.0f, scaled);
}

void scene_init(scene_t* scene, int width, int heigth)
{
    scene_resize(scene, width, heigth);

    scene->camera.radius = 5.0f;
    orbit_init(&scene->camera);

    scene->grid = grid_create();

    gpu_model_init(&scene->gpu_model);

    scene->dirty      = true;
    scene->modelpath  = NULL;
    scene->model_size = 0;
}

void scene_unload(scene_t* scene)
{
    scene->modelpath  = NULL;
    scene->model_size = 0;
    gpu_model_unload(&scene->gpu_model);
    grid_destroy(&scene->grid);
    free(scene->modelpath);
}

void scene_render(scene_t* scene)
{
    gpu_model_render(&scene->gpu_model, scene->projection, scene->camera.view);
    grid_render(&scene->grid, scene->projection, scene->camera.view);
    scene->dirty = false;
}

void scene_load_model(scene_t* scene, const char* modelpath)
{
    // Ingnore if model paths match
    if (scene->modelpath && strcmp(modelpath, scene->modelpath) == 0) return;

    free(scene->modelpath);
    scene->modelpath = malloc(strlen(modelpath) + 1);

    strcpy(scene->modelpath, modelpath);

    gpu_model_unload(&scene->gpu_model);

    model_t model;

    model_init(&model);
    parse_obj(&model, modelpath);
    scene->gpu_model = model_upload(&model);

    vec3 scaled;
    _scale_model_size(scene->gpu_model.min_vertex, scene->gpu_model.max_vertex, scaled);

    grid_build(&scene->grid, (vec3) { 0.0f, scaled[1], 0.0f }, 10, 1.0f, .25f);
    model_free(&model);
    scene->dirty      = true;
    scene->model_size = gpu_model_get_size_mb(&scene->gpu_model);
}

void scene_resize(scene_t* scene, int width, int height)
{
    glViewport(0, 0, width, height);
    glm_perspective(glm_rad(45.0f), width / (float)height, 0.1f, 100.0f, scene->projection);
    scene->window_width  = width;
    scene->window_height = height;
    scene->dirty         = true;
}

void scene_handle_mouse_scroll(scene_t* scene, int yoff)
{
    const float speed    = 0.5f;
    scene->camera.radius = glm_clamp(scene->camera.radius + yoff * speed, 0.0001f, 100.0f);
    orbit_update(&scene->camera);
    scene->dirty = true;
}

void scene_handle_mouse_move(scene_t* scene, float dx, float dy)
{
    const float speed = 0.01f;

    scene->camera.yaw += dx * speed;
    scene->camera.pitch += dy * speed;

    orbit_update(&scene->camera);
    scene->dirty = true;
}

bool scene_is_loaded(scene_t* scene)
{
    return scene->modelpath != NULL;
}