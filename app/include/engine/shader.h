#ifndef __ENGINE_SHADER_H__
#define __ENGINE_SHADER_H__

#include "glad/glad.h"

GLuint load_shader_program(const char* vs_source, const char* fs_source);

#endif // __ENGINE_SHADER_H__