#include "core/grid.h"

#include "glad/glad.h"

#include "engine/shader.h"

static const char* vs_source = "#version 330 core\n"
                               "layout (location = 0) in vec3 aPos;\n"
                               "layout (location = 1) in vec3 aColor;\n"
                               "out vec3 outColor;\n"
                               "uniform mat4 projection;\n"
                               "uniform mat4 view;\n"
                               "void main()\n"
                               "{\n"
                               "    gl_Position = projection * view * vec4(aPos, 1.0);\n"
                               "    outColor = aColor;\n"
                               "}\0";

static const char* fs_source = "#version 330 core\n"
                               "in vec3 outColor;\n"
                               "out vec4 FragColor;\n"
                               "void main()\n"
                               "{\n"
                               "    FragColor = vec4(outColor, 1.0);\n"
                               "}\0";

grid_t grid_create()
{
    return (grid_t) {
        .line_width = 0,
        .line_count = 0,
        .size       = 0,
        .vbo        = 0,
        .cbo        = 0,
        .vao        = 0,
        .program    = 0,
    };
}

void grid_build(grid_t* grid, vec3 center, int size, int line_width, float spacing)
{
    grid->line_width = line_width;
    grid->size       = size;

    float half_size      = size / 2.0f;
    int   lines_per_axis = (int)((size / spacing) + 1);
    grid->line_count     = lines_per_axis * 4; // 2 lines per grid line * 2 axes

    float* vertices = malloc(grid->line_count * 3 * sizeof(float));
    float* colors   = malloc(grid->line_count * 3 * sizeof(float));

    if (!vertices || !colors) {
        free(vertices);
        free(colors);
        return;
    }
    int vertex_count = 0;
    int color_count  = 0;

    // Generate grid lines
    for (float i = -half_size; i <= half_size + spacing * 0.5f; i += spacing) {
        // X axis lines
        vertices[vertex_count++] = -half_size;
        vertices[vertex_count++] = center[1];
        vertices[vertex_count++] = i;

        vertices[vertex_count++] = half_size;
        vertices[vertex_count++] = center[1];
        vertices[vertex_count++] = i;

        // Z axis lines
        vertices[vertex_count++] = i;
        vertices[vertex_count++] = center[1];
        vertices[vertex_count++] = -half_size;

        vertices[vertex_count++] = i;
        vertices[vertex_count++] = center[1];
        vertices[vertex_count++] = half_size;

        if (i == 0) {
            for (int j = 0; j < 2; j++) {
                colors[color_count++] = 0.7f;
                colors[color_count++] = 0.1f;
                colors[color_count++] = 0.1f;
            }
            for (int j = 0; j < 2; j++) {
                colors[color_count++] = 0.1f;
                colors[color_count++] = 0.1f;
                colors[color_count++] = 0.7f;
            }
            continue;
        }

        for (int j = 0; j < 12; j++) {
            colors[color_count++] = 0.4f;
        }
    }

    glGenVertexArrays(1, &grid->vao);
    glBindVertexArray(grid->vao);

    // Vertex buffer
    glGenBuffers(1, &grid->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, grid->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color buffer
    glGenBuffers(1, &grid->cbo);
    glBindBuffer(GL_ARRAY_BUFFER, grid->cbo);
    glBufferData(GL_ARRAY_BUFFER, color_count * sizeof(float), colors, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    free(vertices);
    free(colors);

    grid->program = load_shader_program(vs_source, fs_source);
}
void grid_destroy(grid_t* grid)
{
    glDeleteBuffers(1, &grid->vbo);
    glDeleteVertexArrays(1, &grid->vao);
    glDeleteProgram(grid->program);
}

void grid_render(const grid_t* grid, mat4 projection, mat4 view)
{
    glUseProgram(grid->program);
    glLineWidth((float)grid->line_width);
    glBindVertexArray(grid->vao);

    if (projection) {
        glUniformMatrix4fv(glGetUniformLocation(grid->program, "projection"), 1, GL_FALSE, (float*)projection);
    }
    if (view) {
        glUniformMatrix4fv(glGetUniformLocation(grid->program, "view"), 1, GL_FALSE, (float*)view);
    }
    glDrawArrays(GL_LINES, 0, grid->line_count);
}