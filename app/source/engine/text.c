#define STB_TRUETYPE_IMPLEMENTATION

#include "engine/text.h"
#include "engine/shader.h"

#include "cglm/cglm.h"
#include "glad/glad.h"
#include "stb_truetype.h"

#include <stdlib.h>

typedef struct {
    stbtt_fontinfo  fontinfo;
    unsigned char*  fontbuf;
    unsigned char*  bitmap;
    GLuint          texture;
    GLuint          vao;
    GLuint          vbo;
    GLuint          shader;
    int             bitmap_width;
    int             bitmap_height;
    int             max_vertices;
    int             current_vertex_count;
    int             window_h;
    mat4            projection;
    stbtt_bakedchar cdata[96];
} TextRenderer;

static TextRenderer rd;

static const char* fragment_source = "#version 460\n"
                                     "in vec2 TexCoords;\n"
                                     "out vec4 FragColor;\n"
                                     "uniform sampler2D text;\n"
                                     "uniform vec3 textColor;\n"
                                     "void main() {\n"
                                     "    float alpha = texture(text, TexCoords).r;\n"
                                     "    FragColor = vec4(textColor, alpha);\n"
                                     "};\n";

static const char* vertex_source = "#version 460\n"
                                   "layout (location = 0) in vec4 vertex;\n"
                                   "out vec2 TexCoords;\n"
                                   "uniform mat4 projection;\n"
                                   "void main() {\n"
                                   "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
                                   "    TexCoords = vertex.zw;\n"
                                   "};\n";

void text_init(const char* fontpath, int width, int height)
{
    FILE* fontfile = fopen(fontpath, "rb");
    if (!fontfile) return;

    rd.fontbuf = (unsigned char*)malloc(1 << 20);
    fread(rd.fontbuf, 1, 1 << 20, fontfile);
    fclose(fontfile);

    stbtt_InitFont(&rd.fontinfo, rd.fontbuf, stbtt_GetFontOffsetForIndex(rd.fontbuf, 0));

    rd.max_vertices  = 1024 * 6;
    rd.bitmap_width  = 512;
    rd.bitmap_height = 512;
    rd.bitmap        = (unsigned char*)calloc(rd.bitmap_width * rd.bitmap_height, sizeof(unsigned char));

    float fs = 26.0f;
    stbtt_BakeFontBitmap(rd.fontbuf, 0, fs, rd.bitmap, rd.bitmap_width, rd.bitmap_height, 32, 96, rd.cdata);

    glGenTextures(1, &rd.texture);
    glBindTexture(GL_TEXTURE_2D, rd.texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, rd.bitmap_width, rd.bitmap_height, 0, GL_RED, GL_UNSIGNED_BYTE, rd.bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    rd.shader = load_shader_program(vertex_source, fragment_source);

    glGenVertexArrays(1, &rd.vao);
    glGenBuffers(1, &rd.vbo);
    glBindVertexArray(rd.vao);
    glBindBuffer(GL_ARRAY_BUFFER, rd.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * rd.max_vertices, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glm_ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f, rd.projection);
    rd.window_h = height;
}

void text_update(int width, int height)
{
    glm_ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f, rd.projection);

    rd.window_h = height;
}

void text_draw(const char* text, float x, float y, float scale, vec3 color)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(rd.shader);
    glUniform3f(glGetUniformLocation(rd.shader, "textColor"), color[0], color[1], color[2]);
    glUniformMatrix4fv(glGetUniformLocation(rd.shader, "projection"), 1, GL_FALSE, (float*)rd.projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rd.texture);
    glBindVertexArray(rd.vao);
    glBindBuffer(GL_ARRAY_BUFFER, rd.vbo);

    float penX = x;
    float penY = y;

    float* vertices     = malloc(sizeof(float) * 4 * 6 * strlen(text));
    int    vertex_count = 0;

    while (*text) {
        if (*text >= 32 && *text < 128) {
            stbtt_aligned_quad q;
            // Use flip=1 so Y grows down from top-left
            stbtt_GetBakedQuad(rd.cdata, rd.bitmap_width, rd.bitmap_height, *text - 32, &penX, &penY, &q, 1);

            float blX = q.x0 * scale + x;
            float blY = q.y1 * scale + y;
            float brX = q.x1 * scale + x;
            float brY = q.y1 * scale + y;
            float tlX = q.x0 * scale + x;
            float tlY = q.y0 * scale + y;
            float trX = q.x1 * scale + x;
            float trY = q.y0 * scale + y;

            vertices[vertex_count++] = blX;
            vertices[vertex_count++] = blY;
            vertices[vertex_count++] = q.s0;
            vertices[vertex_count++] = q.t1;

            vertices[vertex_count++] = brX;
            vertices[vertex_count++] = brY;
            vertices[vertex_count++] = q.s1;
            vertices[vertex_count++] = q.t1;

            vertices[vertex_count++] = tlX;
            vertices[vertex_count++] = tlY;
            vertices[vertex_count++] = q.s0;
            vertices[vertex_count++] = q.t0;

            vertices[vertex_count++] = tlX;
            vertices[vertex_count++] = tlY;
            vertices[vertex_count++] = q.s0;
            vertices[vertex_count++] = q.t0;

            vertices[vertex_count++] = brX;
            vertices[vertex_count++] = brY;
            vertices[vertex_count++] = q.s1;
            vertices[vertex_count++] = q.t1;

            vertices[vertex_count++] = trX;
            vertices[vertex_count++] = trY;
            vertices[vertex_count++] = q.s1;
            vertices[vertex_count++] = q.t0;
        }
        text++;
    }

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertex_count, vertices);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count / 4);

    free(vertices);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glUseProgram(0);
}

void text_cleanup()
{
    glDeleteVertexArrays(1, &rd.vao);
    glDeleteBuffers(1, &rd.vbo);
    glDeleteTextures(1, &rd.texture);
    glDeleteProgram(rd.shader);
    free(rd.fontbuf);
    free(rd.bitmap);
}