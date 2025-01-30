#include "glad/glad.h"
#include "log.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "engine/create_window.h"
#include "engine/input.h"
#include "engine/orbit.h"
#define STR_IMPL
#include "engine/string.h"
#include "engine/text.h"

#include "core/scene.h"
#include "parsers/obj.h"

scene_t scene;
int     window_width  = 1280;
int     window_height = 720;

void scroll_callback(GLFWwindow*, double, double yoffset)
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

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        scene_handle_mouse_move(&scene, dx, dy);
    } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        vec2 move = { dx, dy };
        vec2 neg  = { -1.0f, -1.0f };

        float len = glm_vec2_norm(move);

        float direction  = glm_vec2_dot(move, neg);
        float zoom_delta = len * (direction > 0 ? 1.0f : -1.0f);

        scene_handle_mouse_scroll(&scene, zoom_delta * 0.25f);
    }
}

void resize_callback(GLFWwindow*, int width, int height)
{
    window_width  = width;
    window_height = height;
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

    GLFWwindow* window = create_window("fmv", window_width, window_height);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, resize_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetDropCallback(window, drop_callback);

    text_init("c:/code/fov/app/assets/fonts/Silver.ttf", window_width, window_height);
    input_init(window);

    scene_init(&scene, window_width, window_height);

    if (argc >= 2) {
        scene_load_model(&scene, argv[1]);
    }

#ifdef DEBUG_BUILD
    // Load default test model in debug build if no argv is given
    else
    {
        scene_load_model(&scene, "C:/code/fov/test/models/armadillo.obj");
    }
#endif
#ifdef RELEASE_BUILD
    // Log to file in release build
    FILE* log_file = fopen("log.txt", "w+");
    log_add_fp(log_file, LOG_TRACE);
    log_set_quiet(true);
#endif // RELEASE_BUILD

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);

    char   fps_label[16]  = "FPS: N/A";
    char   info_label[64] = "verts: n/a    size: n/a";
    double last_frame = 0.0, this_frame = 0.0, dt = 0.0;
    double fps_timeout = 1.0;

    GLFWcursor* hand_cursor = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    GLFWcursor* norm_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

    while (!glfwWindowShouldClose(window)) {
        this_frame = glfwGetTime();
        dt         = this_frame - last_frame;
        last_frame = this_frame;

        // Exit on escape
        if (get_key(GLFW_KEY_ESCAPE)) break;
        // Reload model
        if (get_key(GLFW_KEY_R) && scene_is_loaded(&scene)) {

            char* temp = e_strdup(scene.modelpath);

            scene_unload(&scene);
            scene_load_model(&scene, temp);
            free(temp);
        }
        // Reset camera
        if (get_key(GLFW_KEY_H) && scene_is_loaded(&scene)) {
            scene.camera.radius = 5.0f;
            orbit_init(&scene.camera);
        }

        // Update fps label
        if (fps_timeout > 1.0) {
            sprintf(fps_label, "FPS: %.2f", (1.0 / dt));
            fps_timeout = 0.0;
        }
        fps_timeout += dt;

        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (scene_is_loaded(&scene)) {
            scene_render(&scene);

            // todo: dont format the string every frame bozo
            sprintf(info_label, "verts: %i  size: %.2fMB", scene.gpu_model.vertex_count, scene.model_size);

            text_draw(info_label, window_width - window_width / 2 - 150, 10, 1.1f, (vec3) { 0.9f, 0.9f, 0.9f });
            text_draw(fps_label, 10, 10, 1.1f, (vec3) { 0.9f, 0.9f, 0.9f });
        } else {
            text_draw("Drop a supported object file.", 10, 10, 2.0f, (vec3) { 1.0f, 1.0f, 1.0f });
        }

        glfwSwapBuffers(window);

        input_update(window);
        glfwPollEvents();
    }

#ifdef RELEASE_BUILD
    fclose(log_file);
#endif // RELEASE_BUILD

    scene_unload(&scene);
    text_cleanup();

    glfwDestroyCursor(hand_cursor);
    glfwDestroyCursor(norm_cursor);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}