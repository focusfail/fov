#include "core/model.h"

#include <stdlib.h>

#include "glad/glad.h"
#include "log.h"

#include "engine/shader.h"

static const char* vs_source_normals_texcoords = "#version 460 core\n"
                                                 "layout (location = 0) in dvec3 aPos;\n"
                                                 "layout (location = 1) in vec3 aNormal;\n"
                                                 "layout (location = 2) in vec2 aTexcoord;\n"
                                                 "out vec3 FragPos;\n"
                                                 "out vec3 Normal;\n"
                                                 "uniform mat4 uProj;\n"
                                                 "uniform mat4 uView;\n"
                                                 "uniform mat4 uModel;\n"
                                                 "uniform vec3 uModelMin;\n"
                                                 "uniform vec3 uModelMax;\n"
                                                 "void main() {\n"
                                                 "    vec3 center = (uModelMax + uModelMin) * 0.5;\n"
                                                 "    vec3 extent = (uModelMax - uModelMin) * 0.5;\n"
                                                 "    float maxExtent = max(max(extent.x, extent.y), extent.z);\n"
                                                 "    vec3 normalized = (vec3(aPos) - center) / maxExtent;\n"
                                                 "    vec3 scaled = normalized * 1.0f;\n"
                                                 "    gl_Position = uProj * uView * uModel * vec4(scaled, 1.0);\n"
                                                 "    FragPos = vec3(uModel * vec4(scaled, 1.0));\n"
                                                 "    Normal = mat3(transpose(inverse(uModel))) * aNormal;\n"
                                                 "}\n";

static const char* fs_source_normals_texcoords
    = "#version 460 core\n"
      "in vec3 FragPos;\n"
      "in vec3 Normal;\n"
      "out vec4 FragColor;\n"
      "void main() {\n"
      "    // Material properties\n"
      "    vec3 baseColor = vec3(0.8, 0.8, 0.8);\n"
      "    float ambientStrength = 0.2;\n"
      "    float specularStrength = 0.5;\n"
      "    float shininess = 32.0;\n"
      "    // Light properties\n"
      "    vec3 lightPos = vec3(2.0, 2.0, 2.0);\n"
      "    vec3 viewPos = vec3(0.0, 0.0, 5.0);\n"
      "    // Ambient\n"
      "    vec3 ambient = ambientStrength * baseColor;\n"
      "    // Diffuse\n"
      "    vec3 norm = normalize(Normal);\n"
      "    vec3 lightDir = normalize(lightPos - FragPos);\n"
      "    float diff = max(dot(norm, lightDir), 0.0);\n"
      "    vec3 diffuse = diff * baseColor;\n"
      "    // Specular\n"
      "    vec3 viewDir = normalize(viewPos - FragPos);\n"
      "    vec3 reflectDir = reflect(-lightDir, norm);\n"
      "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n"
      "    vec3 specular = specularStrength * spec * vec3(1.0);\n"
      "    // Combine results\n"
      "    vec3 result = ambient + diffuse + specular;\n"
      "    FragColor = vec4(result, 1.0);\n"
      "}\n";

static const char* vs_source = "#version 460 core\n"
                               "layout (location = 0) in dvec3 aPos;\n"
                               "out vec3 FragPos;\n"
                               "uniform mat4 uProj;\n"
                               "uniform mat4 uView;\n"
                               "uniform mat4 uModel;\n"
                               "uniform vec3 uModelMin;\n"
                               "uniform vec3 uModelMax;\n"
                               "void main() {\n"
                               "    vec3 center = (uModelMax + uModelMin) * 0.5;\n"
                               "    vec3 extent = (uModelMax - uModelMin) * 0.5;\n"
                               "    float maxExtent = max(max(extent.x, extent.y), extent.z);\n"
                               "    vec3 normalized = (vec3(aPos) - center) / maxExtent;\n"
                               "    vec3 scaled = normalized * 1.0f;\n"
                               "    gl_Position = uProj * uView * uModel * vec4(scaled, 1.0);\n"
                               "    FragPos = vec3(uModel * vec4(scaled, 1.0));\n"
                               "}\n";

static const char* fs_source = "#version 460 core\n"
                               "in vec3 FragPos;\n"
                               "out vec4 FragColor;\n"
                               "void main() {\n"
                               "    // Calculate face normal using derivatives\n"
                               "    vec3 dx = dFdx(FragPos);\n"
                               "    vec3 dy = dFdy(FragPos);\n"
                               "    vec3 normal = normalize(cross(dx, dy));\n"
                               "    vec3 lightDir = normalize(vec3(1.0, 10.0, -1.0));\n"
                               "    float diff = max(dot(normal, lightDir), 0.0);\n"
                               "    vec3 baseColor = vec3(0.8, 0.8, 0.8);\n"
                               "    vec3 ambient = 0.2 * baseColor;\n"
                               "    vec3 diffuse = diff * baseColor;\n"
                               "    FragColor = vec4(ambient + diffuse, 1.0);\n"
                               "}\n";

void model_init(model_t* m)
{
    m->vertex_count = 0;
    m->normal_count = 0;
    m->texcrd_count = 0;
    m->indice_count = 0;

    m->indices  = malloc(MAX_INDICES * sizeof(unsigned int));
    m->vertices = malloc(MAX_VERTICES * 3 * sizeof(double));
    m->texcrds  = malloc(MAX_VERTICES * 2 * sizeof(float));
    m->normals  = malloc(MAX_VERTICES * 3 * sizeof(float));
}

void model_free(model_t* m)
{
    free(m->indices);
    free(m->vertices);
    free(m->texcrds);
    free(m->normals);
}

gpu_model_t model_upload(model_t* m)
{
    gpu_model_t g;

    vec3 min = { INFINITY, INFINITY, INFINITY };
    vec3 max = { -INFINITY, -INFINITY, -INFINITY };

    for (int i = 0; i < m->vertex_count; i += 3) {
        min[0] = fmin(min[0], m->vertices[i]);
        min[1] = fmin(min[1], m->vertices[i + 1]);
        min[2] = fmin(min[2], m->vertices[i + 2]);

        max[0] = fmax(max[0], m->vertices[i]);
        max[1] = fmax(max[1], m->vertices[i + 1]);
        max[2] = fmax(max[2], m->vertices[i + 2]);
    }

    glm_vec3_copy(min, g.min_vertex);
    glm_vec3_copy(max, g.max_vertex);

    glm_mat4_identity(g.model);
    glGenVertexArrays(1, &g.vao);
    glBindVertexArray(g.vao);

    log_info("Vertex count: %i", m->vertex_count);

    // Vertex buffer object
    glGenBuffers(1, &g.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g.vbo);
    glBufferData(GL_ARRAY_BUFFER, m->vertex_count * sizeof(double), m->vertices, GL_STATIC_DRAW);
    glVertexAttribLPointer(0, 3, GL_DOUBLE, 0, NULL);
    glEnableVertexAttribArray(0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        log_error("OpenGL error during vertex buffer upload: 0x%x", err);
        return (gpu_model_t) { 0 };
    }

    if (m->normal_count > 0) {
        // Normal buffer object
        glGenBuffers(1, &g.nbo);
        glBindBuffer(GL_ARRAY_BUFFER, g.nbo);
        glBufferData(GL_ARRAY_BUFFER, m->normal_count * sizeof(float), m->normals, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);

        err = glGetError();
        if (err != GL_NO_ERROR) {
            log_error("OpenGL error during normal buffer upload: 0x%x", err);
            return (gpu_model_t) { 0 };
        }
    }

    if (m->texcrd_count > 0) {
        // Texcoord buffer object
        glGenBuffers(1, &g.tbo);
        glBindBuffer(GL_ARRAY_BUFFER, g.tbo);
        glBufferData(GL_ARRAY_BUFFER, m->texcrd_count * sizeof(float), m->texcrds, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);
    }

    err = glGetError();
    if (err != GL_NO_ERROR) {
        log_error("OpenGL error during texcoord buffer upload: 0x%x", err);
        return (gpu_model_t) { 0 };
    }

    // Element buffer object
    glGenBuffers(1, &g.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->indice_count * sizeof(unsigned int), m->indices, GL_STATIC_DRAW);

    err = glGetError();
    if (err != GL_NO_ERROR) {
        log_error("OpenGL error during element buffer upload: 0x%x", err);
        return (gpu_model_t) { 0 };
    }

    g.vertex_count = m->vertex_count / 3;
    g.indice_count = m->indice_count;

    if (m->texcrd_count > 0 && m->normal_count > 0 && !FORCE_SIMPLE_SHADER) {
        g.program = load_shader_program(vs_source_normals_texcoords, fs_source_normals_texcoords);
        log_info("Using normal & texcoord shader.");
    } else {
        g.program = load_shader_program(vs_source, fs_source);
        log_info("Using default shader.");
    }

    glBindVertexArray(0);

    return g;
}

void model_render(const gpu_model_t* g, mat4 proj, mat4 view)
{
    glUseProgram(g->program);
    glBindVertexArray(g->vao);

    if (proj) {
        glUniformMatrix4fv(glGetUniformLocation(g->program, "uProj"), 1, GL_FALSE, (float*)proj);
    }

    if (view) {
        glUniformMatrix4fv(glGetUniformLocation(g->program, "uView"), 1, GL_FALSE, (float*)view);
    }

    // glm_rotate(g->model, 90.0f, (vec3) { 1.0f, 0.0f, 0.0f });
    glUniformMatrix4fv(glGetUniformLocation(g->program, "uModel"), 1, GL_FALSE, (float*)g->model);
    glUniform3fv(glGetUniformLocation(g->program, "uModelMin"), 1, (float*)g->min_vertex);
    glUniform3fv(glGetUniformLocation(g->program, "uModelMax"), 1, (float*)g->max_vertex);
    glDrawElements(GL_TRIANGLES, g->indice_count, GL_UNSIGNED_INT, 0);

    glUseProgram(0);
    glBindVertexArray(0);
}

float model_get_size_mb(const model_t* m)
{
    float mbs = ((m->vertex_count * sizeof(double) +       //
                  m->indice_count * sizeof(unsigned int) + //
                  m->normal_count * sizeof(float) +        //
                  m->texcrd_count * sizeof(float))         //
                 / (1024.0f * 1024.0f));

    return mbs;
}