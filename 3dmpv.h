// GNU GPLv3
// Copyright (c) 2020 v0idv0id - Martin Willner - lvslinux@gmail.com


#ifndef __MPVIDEOCUBE_H
#define __MPVIDEOCUBE_H

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <mpv/client.h>
#include <mpv/render_gl.h>

#include <shader.h>
#include <camera.h>

int window_width = 800;
int window_height = 600;
int fbo_width = 800;
int fbo_height = 600;

GLFWwindow *window = NULL;
mpv_handle *mpv;
mpv_render_context *mpv_ctx;

unsigned int video_framebuffer;
unsigned int video_textureColorbuffer;

unsigned int screen_framebuffer;
unsigned int screen_textureColorbuffer;
int nonAffine(float *vertex);

float deltaTime, lastFrame;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

static void *get_proc_address(void *ctx, const char *name);
void processGLFWInput(GLFWwindow *window);

float cubeVertices[] = {
    // positions          // texture Coords
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

// float quadVertices[] = {
//     // positions         // texCoords
//     -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//     -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
//     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

//     -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
//     1.0f, 1.0f, 0.0f, 1.0f, 1.0f};


float quadVertices[36];
float quadVertices_orig[36] = {
    // positions 3D        // texCoords 3D      // texCoords 3D original
    -1.0f, -1.0f, 0.0f,     0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
     1.0f, -1.0f, 0.0f,     1.0f, 0.0f, 1.0f,   1.0f, 0.0f, 1.0f,
     1.0f,  1.0f, 0.0f,     1.0f, 1.0f, 1.0f,   1.0f, 1.0f, 1.0f,
    -1.0f,  1.0f, 0.0f,     0.0f, 1.0f, 1.0f,   0.0f, 1.0f, 1.0f,
};
unsigned int quadIndices[] = {
    0, 1, 3, 1, 2, 3};

int _id=0;

static void on_mpv_render_update(void *ctx);
static void on_mpv_events(void *ctx);



#endif