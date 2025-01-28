#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "engine/arcball.h"
#include "engine/create_window.h"
#include "engine/input.h"
#include "engine/text.h"

#include "parsers/obj.h"

arcball_t arcball;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    arcball_zoom(&arcball, 1.0f + (yoffset * 0.1));
}

// ...existing code...

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    static double last_xpos = 0, last_ypos = 0;
    float         dx = (float)(last_xpos - xpos);
    float         dy = (float)(last_ypos - ypos);
    last_xpos        = xpos;
    last_ypos        = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS) return;

    // Update yaw/pitch directly
    arcball.yaw += dx * 0.25f;
    arcball.pitch += dy * 0.25f;

    // Clamp pitch
    if (arcball.pitch > 85.0f) arcball.pitch = 85.0f;
    if (arcball.pitch < -85.0f) arcball.pitch = -85.0f;

    // Normalize yaw
    if (arcball.yaw < -180.0f) arcball.yaw += 360.0f;
    if (arcball.yaw > 180.0f) arcball.yaw -= 360.0f;

    // Rebuild rotation
    glm_mat4_identity(arcball.rotation);
    glm_rotate_x(arcball.rotation, glm_rad(arcball.pitch), arcball.rotation);
    glm_rotate_y(arcball.rotation, glm_rad(arcball.yaw), arcball.rotation);

    arcball_update_view_matrix(&arcball);
}

void resize_callback(GLFWwindow* window, int width, int height)
{
    text_update(width, height);
}

// ...existing code...

int main(int argc, char const* argv[])
{
    const char* fp = "c:/code/fmv/test/models/LBody.obj";

    if (argc >= 2) {
        fp = argv[1];
    }

    GLFWwindow* window = create_window("fmv", 1280, 720);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, resize_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    text_init("c:/code/fmv/app/assets/fonts/Silver.ttf", 1280, 720);
    input_init(window);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    arcball_init(&arcball);

    model_t model;

    clock_t t;
    t = clock();

    model_init(&model);
    parse_obj(&model, fp);
    unsigned int model_size = model_get_size_mb(&model);
    gpu_model_t  gpu_model  = model_upload(&model);

    t         = clock() - t;
    double tt = ((double)t) / CLOCKS_PER_SEC;
    printf("OBJ parse took %f seconds \n", tt);

    double last_frame = 0.0, this_frame = 0.0, dt = 0.0;
    char   fps_chars[16]       = "FPS: N/A";
    double fps_timeout         = 1.0;
    char   info_bar_chars[128] = "verts: n/a    tris: n/a    size: %uMB";
    sprintf(info_bar_chars, "verts: %u    tris: %u    size: %uMB", gpu_model.vertex_count, gpu_model.indice_count / 3,
            model_size);

    mat4 proj;
    glm_perspective(glm_rad(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f, proj);

    GLFWcursor* hand_cursor = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
    GLFWcursor* norm_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

    bool wireframe = false;
    bool edges     = false;
    bool points    = false;

    while (!glfwWindowShouldClose(window)) {
        this_frame = glfwGetTime();
        dt         = this_frame - last_frame;
        last_frame = this_frame;

        if (get_key(GLFW_KEY_ESCAPE)) break;
        if (get_key(GLFW_KEY_W)) wireframe ^= 1;
        if (get_key(GLFW_KEY_E)) edges ^= 1;
        if (get_key(GLFW_KEY_P)) points ^= 1;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
            glfwSetCursor(window, hand_cursor);

        } else {
            glfwSetCursor(window, norm_cursor);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        int wwidth, wheight;
        glfwGetWindowSize(window, &wwidth, &wheight);

        { // FPS
            if (fps_timeout > 1.0) {
                sprintf(fps_chars, "FPS: %.2f", (1.0 / dt));
                fps_timeout = 0.0;
            }
            glfwSetWindowTitle(window, fps_chars);
            fps_timeout += dt;
        }

        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (edges) {
            model_render(&gpu_model, proj, arcball.view);
        }

        if (wireframe || edges) {
            glLineWidth(5.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        if (points) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        }
        if (!edges && !wireframe && !points) {
            glPointSize(5.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        model_render(&gpu_model, proj, arcball.view);

        if (wireframe || edges) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        text_draw(info_bar_chars, 10, wheight - 15, 0.1f, (vec3) { 1.0f, 1.0f, 1.0f });

        glfwSwapBuffers(window);
        input_update(window);
        glfwPollEvents();
    }

    model_free(&model);
    text_cleanup();

    glfwDestroyCursor(hand_cursor);
    glfwDestroyCursor(norm_cursor);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}