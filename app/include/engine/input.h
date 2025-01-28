#ifndef __INPUT_H__
#define __INPUT_H__

#include <stdbool.h>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

void input_key_callback(GLFWwindow* w, int key, int sc, int ac, int mods);

void input_init(GLFWwindow* window);
void input_update(GLFWwindow* window);
bool get_key(unsigned int key);

#endif // __INPUT_H__