#ifndef __SCENE_H__
#define __SCENE_H__

#include "core/model.h"
#include "engine/orbit.h"

typedef struct {
    const char* modelpath;
    orbit_cam_t camera;
    gpu_model_t gpu_model;
    int         window_height;
    int         window_width;
    mat4        projection;
    bool        dirty;
} scene_t;

void scene_init(scene_t* scene, int width, int height);
void scene_free(scene_t* scene);
void scene_render(scene_t* scene);
void scene_load_model(scene_t* scene, const char* modelpath);
void scene_resize(scene_t* scene, int width, int height);
void scene_handle_mouse_scroll(scene_t* scene, int yoff);
void scene_handle_mouse_move(scene_t* scene, float dx, float dy);

#endif // __SCENE_H__