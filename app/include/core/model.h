#ifndef __MODEL_H__
#define __MODEL_H__

#include "cglm/cglm.h"

#define MAX_VERTICES 1000000
#define MAX_INDICES 1000000
#define FORCE_SIMPLE_SHADER 1

typedef struct {
    // unsigned int indices[MAX_INDICES * 3];
    // float        vertices[MAX_VERTICES * 3];
    // float        normals[MAX_VERTICES * 3];
    // float        texcrds[MAX_VERTICES * 2];
    unsigned int* indices;
    double*       vertices;
    float*        normals;
    float*        texcrds;
    int           indice_count;
    int           vertex_count;
    int           normal_count;
    int           texcrd_count;
} model_t;

typedef struct {
    unsigned int vao;
    unsigned int vbo;
    unsigned int nbo;
    unsigned int tbo;
    unsigned int ebo;
    unsigned int program;
    int          vertex_count;
    int          indice_count;
    vec3         min_vertex;
    vec3         max_vertex;
    mat4         model;
} gpu_model_t;

void        model_init(model_t* model);
void        model_free(model_t* model);
float       model_get_size_mb(const model_t* model);
gpu_model_t model_upload(model_t* model);

void model_render(const gpu_model_t* model, mat4 proj, mat4 view);

#endif // __MODEL_H__