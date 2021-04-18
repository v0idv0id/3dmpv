// GNU GPLv3
// Copyright (c) 2020 v0idv0id - Martin Willner - lvslinux@gmail.com

#ifndef __3DMPV_H
#define __3DMPV_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unistd.h>
#include <mpv/client.h>
#include <mpv/render_gl.h>
#include <cstring>
#include <shader.h>
#include <thread>

int window_width = 800;
int window_height = 600;

int wakeup = 0;
GLFWwindow *window = NULL;
GLFWwindow *window2 = NULL;

#define MAXINSTANCES 10

mpv_handle *mpv[MAXINSTANCES];

mpv_render_context *mpv_ctx[MAXINSTANCES];

mpv_opengl_fbo mpv_fbo[MAXINSTANCES];

mpv_render_param params_fbo[MAXINSTANCES][3];

unsigned int video_framebuffer[MAXINSTANCES];

unsigned int video_textureColorbuffer[MAXINSTANCES];

int active_instances=0;
float currentFrame;
bool animation=true;
bool showfx=true;
float vignette=0;
float trans_alpha=0;
unsigned int cubeVAO, cubeVBO;
unsigned int quadVAO, quadVBO, quadEBO;

int nonAffine(float *vertex);

float deltaTime, lastFrame;


static void *get_proc_address(void *ctx, const char *name);
void processGLFWInput(GLFWwindow *window);
void texture_task(std::string);

float quadVertices[MAXINSTANCES][36];
float quadVertices_orig[36] = {
    // positions 3D        // texCoords 3D      // texCoords 3D original
    -1.0f,
    -1.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    1.0f,
    -1.0f,
    0.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    1.0f,
    -1.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
};
unsigned int quadIndices[] = {
    0, 1, 3, 1, 2, 3};

int _id = 0;
int activecorner[MAXINSTANCES];
float inline inCircleN(float x, float y, float r, float x0, float y0);

static void on_mpv_render_update(void *ctx);
static void on_mpv_events(void *ctx);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);

#endif