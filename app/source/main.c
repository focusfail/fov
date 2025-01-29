#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "engine/create_window.h"
#include "engine/input.h"
#include "engine/orbit.h"
#include "engine/text.h"

#include "core/scene.h"
#include "parsers/obj.h"

scene_t scene;

void scroll_callback(GLFWwindow* _window, double _xoffset, double yoffset)
{
    scene_handle_mouse_scroll(&scene, yoffset);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    static double last_xpos = 0, last_ypos = 0;

    float dx  = (float)(last_xpos - xpos);
    float dy  = (float)(last_ypos - ypos);
    last_xpos = xpos;
    last_ypos = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) return;

    scene_handle_mouse_move(&scene, dx, dy);
}

void resize_callback(GLFWwindow*, int width, int height)
{
    text_update(width, height);
    scene_resize(&scene, width, height);
}

void drop_callback(GLFWwindow*, int count, const char** paths)
{
    if (count > 0) {
        scene_load_model(&scene, paths[0]);
    }
}

int main(int argc, char const* argv[])
{
    const char* fp = "c:/code/fmv/test/models/LBody.obj";

    if (argc >= 2) {
        fp = argv[1];
    }

    int window_width  = 1280;
    int window_height = 720;

    GLFWwindow* window = create_window("fmv", window_width, window_height);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, resize_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetDropCallback(window, drop_callback);

    text_init("c:/code/fmv/app/assets/fonts/Silver.ttf", window_width, window_height);
    input_init(window);

    scene_init(&scene, window_width, window_height);
    scene_load_model(&scene, fp);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);

    double last_frame = 0.0, this_frame = 0.0, dt = 0.0;
    char   fps_chars[16] = "FPS: N/A";
    double fps_timeout   = 1.0;

    GLFWcursor* hand_cursor = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    GLFWcursor* norm_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

    while (!glfwWindowShouldClose(window)) {
        this_frame = glfwGetTime();
        dt         = this_frame - last_frame;
        last_frame = this_frame;

        if (get_key(GLFW_KEY_ESCAPE)) break;

        // Update fps label
        if (fps_timeout > 1.0) {
            sprintf(fps_chars, "FPS: %.2f", (1.0 / dt));
            fps_timeout = 0.0;
        }
        glfwSetWindowTitle(window, fps_chars);
        fps_timeout += dt;

        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene_render(&scene);

        glfwSwapBuffers(window);

        input_update(window);
        glfwPollEvents();
    }

    scene_free(&scene);
    text_cleanup();

    glfwDestroyCursor(hand_cursor);
    glfwDestroyCursor(norm_cursor);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}