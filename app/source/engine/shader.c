#include "engine/shader.h"

#include "log.h"

GLuint _shader_compile(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        log_error("Shader compilation failed: %s", info_log);
    }

    return shader;
}

GLuint load_shader_program(const char* vs_source, const char* fs_source)
{
    GLuint vs = _shader_compile(GL_VERTEX_SHADER, vs_source);
    GLuint fs = _shader_compile(GL_FRAGMENT_SHADER, fs_source);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    GLint success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(prog, 512, NULL, info_log);
        log_error("Shader program linking failed: %s", info_log);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return prog;
}