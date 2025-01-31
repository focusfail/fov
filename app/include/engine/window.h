#ifndef __ENGINE_WINDOW_H__
#define __ENGINE_WINDOW_H__

#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "log.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <dwmapi.h>
#endif

#include <stdbool.h>

static GLFWwindow* create_window(const char* title, int width, int height)
{
    if (!glfwInit()) {
        log_error("Failed to initialize GLFW");
        return NULL;
    }

    GLFWwindow* window;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, 1);
    window = glfwCreateWindow(width, height, title, NULL, NULL);

    if (!window) {
        log_error("Failed to create window");
        return NULL;
    }

    glfwMakeContextCurrent(window);

#ifdef _WIN32
    HWND hwnd  = glfwGetWin32Window(window);
    BOOL value = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
#endif

    if (!gladLoadGL()) {
        log_error("Failed to initialize glad");
        glfwDestroyWindow(window);
        glfwTerminate();
        return NULL;
    }

    return window;
}

#endif // __ENGINE_WINDOW_H__