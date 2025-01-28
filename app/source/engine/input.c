#include "engine/input.h"

#include <stdio.h>
#include <string.h>

static bool this_keys[GLFW_KEY_LAST] = { false };
static bool prev_keys[GLFW_KEY_LAST] = { false };

void input_key_callback(GLFWwindow* w, int key, int sc, int ac, int mods)
{
    if (key >= 0 && key < GLFW_KEY_LAST) {
        if (ac == GLFW_PRESS) {
            this_keys[key] = true;
        } else if (ac == GLFW_RELEASE) {
            this_keys[key] = false;
        }
    }
}

void input_init(GLFWwindow* window)
{
    glfwSetKeyCallback(window, input_key_callback);
}

void input_update(GLFWwindow* window)
{
    memcpy(prev_keys, this_keys, sizeof(bool) * GLFW_KEY_LAST);
    memcpy(this_keys, prev_keys, sizeof(bool) * GLFW_KEY_LAST);
}

bool get_key(unsigned int k)
{
    return this_keys[k] && !prev_keys[k];
}