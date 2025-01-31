#include "glad/glad.h"
#include "log.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core/grid.h"
#include "engine/input.h"
#include "engine/orbit.h"
#include "engine/window.h"
#define STR_IMPL
#include "engine/string.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include "nuklear.h"
#define NK_GLFW_GL4_IMPLEMENTATION
#include "nuklear_glfw_gl4.h"

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
    struct nk_context* ctx;

    GLFWwindow* window = create_window("fov", window_width, window_height);

    // nuklear setup
    ctx = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS, 512 * 1024, 128 * 1024);
    struct nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&atlas);
    struct nk_font* large_font = nk_font_atlas_add_default(atlas, 46, 0);
    struct nk_font* norm_font  = nk_font_atlas_add_default(atlas, 20, 0);
    nk_glfw3_font_stash_end();
    nk_style_set_font(ctx, &norm_font->handle);

    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, resize_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetDropCallback(window, drop_callback);

    input_init(window);

    scene_init(&scene, window_width, window_height);

    if (argc >= 2) {
        scene_load_model(&scene, argv[1]);
    }

#ifdef DEBUG_BUILD
    // Load default test model in debug build if no argv is given
    else
    {
        // scene_load_model(&scene, "C:/code/fov/test/models/armadillo.obj");
    }
#endif
#ifdef RELEASE_BUILD
    // Log to file in release build
    FILE* log_file = fopen("fov.log", "w+");
    log_add_fp(log_file, LOG_TRACE);
    log_set_quiet(true);
#endif // RELEASE_BUILD

    float  fps         = 0.0f;
    double dt          = 0.0;
    double last_frame  = 0.0;
    double this_frame  = 0.0;
    double fps_timeout = 1.0;

    GLFWcursor* hand_cursor = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    GLFWcursor* norm_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

    while (!glfwWindowShouldClose(window)) {
        input_update(window);
        glfwPollEvents();

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
            fps         = (float)(1.0 / dt);
            fps_timeout = 0.0;
        }
        fps_timeout += dt;

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glDepthFunc(GL_LESS);
        glViewport(0, 0, window_width, window_height);
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        nk_glfw3_new_frame();

        if (scene_is_loaded(&scene)) {

            scene_render(&scene);

            if (nk_begin(ctx, "invisible_window", nk_rect(0, 0, (float)window_width, (float)window_height),
                         NK_WINDOW_NO_INPUT | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND
                             | NK_WINDOW_NOT_INTERACTIVE))
            {
                ctx->style.window.background       = nk_rgba(0, 0, 0, 0);
                ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.window.border           = 0;
                ctx->style.window.padding          = nk_vec2(0, 0);
                ctx->style.window.spacing          = nk_vec2(0, 0);

                nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);
                nk_layout_row_push(ctx, 0.5f);
                nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "fps: %.1f", fps);
                nk_layout_row_push(ctx, 0.5f);
                nk_labelf(ctx, NK_TEXT_ALIGN_RIGHT, "verts: %i  size: %.2fMB", scene.gpu_model.vertex_count,
                          scene.model_size);
                nk_layout_row_end(ctx);
            }
            nk_end(ctx);

        } else {
            if (nk_begin(ctx, "invisible_window", nk_rect(0, 0, (float)window_width, (float)window_height),
                         NK_WINDOW_NO_INPUT | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND
                             | NK_WINDOW_NOT_INTERACTIVE))
            {
                ctx->style.window.background       = nk_rgba(0, 0, 0, 0);
                ctx->style.window.fixed_background = nk_style_item_color(nk_rgba(0, 0, 0, 0));
                ctx->style.window.border           = 0;
                ctx->style.window.padding          = nk_vec2(0, 0);
                ctx->style.window.spacing          = nk_vec2(0, 0);

                // Set a larger font
                nk_style_set_font(ctx, &large_font->handle);

                // Center the text vertically and horizontally
                nk_layout_space_begin(ctx, NK_STATIC, window_height, 1);
                nk_layout_space_push(ctx, nk_rect(0, window_height / 2 - 15, window_width, 30));
                nk_label(ctx, "Drop a supported 3d model file.", NK_TEXT_ALIGN_CENTERED);
                nk_layout_space_end(ctx);

                nk_style_set_font(ctx, &norm_font->handle);
            }
            nk_end(ctx);
        }

        nk_glfw3_render(NK_ANTI_ALIASING_ON);
        glfwSwapBuffers(window);
    }

#ifdef RELEASE_BUILD
    fclose(log_file);
#endif // RELEASE_BUILD

    scene_unload(&scene);

    glfwDestroyCursor(hand_cursor);
    glfwDestroyCursor(norm_cursor);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}