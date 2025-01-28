#include "engine/draw.h"
#include "engine/shader.h"

#include "glad/glad.h"

#include <string.h>

typedef struct {
    int    vertex_count;
    GLuint vbo, vao, program;
    float* vertices;
    int    max_vertices;
} Renderer;

static Renderer rd;

static const char* vs = "#version 460\n"
                        "layout(location = 0) in vec2 aPos;\n"
                        "layout(location = 1) in vec3 aColor;\n"
                        "out vec3 ourColor;\n"
                        "uniform mat4 uProjection;\n"
                        "void main()\n"
                        "{\n"
                        "    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);\n"
                        "    ourColor = aColor;\n"
                        "}\n";

static const char* fs = "#version 460\n"
                        "in vec3 ourColor;\n"
                        "out vec4 FragColor;\n"
                        "void main()\n"
                        "{\n"
                        "    FragColor = vec4(ourColor, 1.0);\n"
                        "}\n";

void draw_init(int screen_width, int screen_height)
{
    rd.program = load_shader_program(vs, fs);
    glGenVertexArrays(1, &rd.vao);
    glGenBuffers(1, &rd.vbo);

    rd.max_vertices = 1024;                                                // Adjust as needed
    rd.vertices     = (float*)malloc(rd.max_vertices * 5 * sizeof(float)); // 2 for position, 3 for color
    rd.vertex_count = 0;

    glBindVertexArray(rd.vao);
    glBindBuffer(GL_ARRAY_BUFFER, rd.vbo);
    glBufferData(GL_ARRAY_BUFFER, rd.max_vertices * 5 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Calculate orthographic projection matrix
    mat4 ortho;
    glm_ortho(0.0f, (float)screen_width, (float)screen_height, 0.0f, -1.0f, 1.0f, ortho);

    // Pass orthographic projection matrix to the shader
    glUseProgram(rd.program);
    GLint projLoc = glGetUniformLocation(rd.program, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, (const GLfloat*)ortho);
}

void draw_cleanup()
{
    glDeleteVertexArrays(1, &rd.vao);
    glDeleteBuffers(1, &rd.vbo);
    glDeleteProgram(rd.program);
    free(rd.vertices);
}

void draw_rectangle(float x, float y, float width, float height, vec3 color)
{
    if (rd.vertex_count + 6 > rd.max_vertices) {
        // Not enough space, flush the buffer
        draw_flush();
    }

    float vertices[] = { x,        y,        color[0],  color[1],   color[2], x + width, y,          color[0],
                         color[1], color[2], x + width, y + height, color[0], color[1],  color[2],

                         x,        y,        color[0],  color[1],   color[2], x + width, y + height, color[0],
                         color[1], color[2], x,         y + height, color[0], color[1],  color[2] };

    memcpy(rd.vertices + rd.vertex_count * 5, vertices, sizeof(vertices));
    rd.vertex_count += 6;
}

void draw_flush()
{
    if (rd.vertex_count == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, rd.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, rd.vertex_count * 5 * sizeof(float), rd.vertices);

    glUseProgram(rd.program);
    glBindVertexArray(rd.vao);
    glDrawArrays(GL_TRIANGLES, 0, rd.vertex_count);

    rd.vertex_count = 0;
}