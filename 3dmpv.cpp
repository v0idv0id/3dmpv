// GNU GPLv3
// Copyright (c) 2020 v0idv0id - Martin Willner - lvslinux@gmail.com

#include "3dmpv.h"

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " videofilename" << std::endl;
        return -1;
    }
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // GLFWmonitor *primary = glfwGetPrimaryMonitor();
    // const GLFWvidmode *mode = glfwGetVideoMode(primary);
    // window_width = mode->width;
    // window_height = mode->height;
    // if ((window = glfwCreateWindow(window_width, window_height, "3dmpv",  glfwGetPrimaryMonitor(), NULL)) == NULL) // for fullscreen windows
    if ((window = glfwCreateWindow(window_width, window_height, "3dmpv", NULL, NULL)) == NULL)
    {
        std::cout << "ERROR::GLFW::Failed to create window" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ERROR::GLAD::Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // MPV initialization and configuration
    mpv = mpv_create();
    if (mpv_initialize(mpv) < MPV_ERROR_SUCCESS)
    {
        std::cout << "ERROR::MPV::Failed to initialize mpv" << std::endl;
        return -1;
    }
    mpv_request_log_messages(mpv, "debug");

    mpv_opengl_init_params opengl_init_params{
        get_proc_address,
        nullptr,
        nullptr};

    int adv{1};

    mpv_render_param render_param[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &opengl_init_params},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &adv},
        {MPV_RENDER_PARAM_BLOCK_FOR_TARGET_TIME, (int)0},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    if (mpv_render_context_create(&mpv_ctx, mpv, render_param) < MPV_ERROR_SUCCESS)
    {
        std::cout << "ERROR::MPV::Failed to create MPV render context" << std::endl;
        return -1;
    }

    mpv_set_wakeup_callback(mpv, on_mpv_events, NULL);
    mpv_render_context_set_update_callback(mpv_ctx, on_mpv_render_update, NULL);

    const char *cmd[] = {"loadfile", argv[1], NULL};
    mpv_command(mpv, cmd);
    mpv_set_option_string(mpv, "gpu-api", "opengl");
    mpv_set_option_string(mpv, "hwdec", "auto");
    mpv_set_option_string(mpv, "vd-lavc-dr", "yes");
    mpv_set_option_string(mpv, "loop", "");
    mpv_set_option_string(mpv, "load-unsafe-playlists", "");
    mpv_set_option_string(mpv, "terminal", "yes");

    // SHADER creation

    Shader *screenShader = new Shader("shaders/screen_vs.glsl", "shaders/screen_fs.glsl");
    Shader *fxShader = new Shader("shaders/fx_vs.glsl", "shaders/fx_fs.glsl");
    Shader pointShader("shaders/point_vs.glsl", "shaders/point_fs.glsl");

    memcpy(quadVertices, quadVertices_orig, sizeof(quadVertices));

    // SCREEN QUAD
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices, GL_STATIC_DRAW);
    _id = 0;
    glEnableVertexAttribArray(_id);                                                  // vertex coordinates layout(location = 0)
    glVertexAttribPointer(_id, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0); // vertex coordinates
    _id = 1;
    glEnableVertexAttribArray(_id);                                                                    //texture coord,  layout(location = 1)
    glVertexAttribPointer(_id, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float))); //texture
    _id = 2;
    glEnableVertexAttribArray(_id);                                                                    //texture2 coord, layout(location = 2)
    glVertexAttribPointer(_id, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(6 * sizeof(float))); //texture2
    glBindVertexArray(0);                                                                              // unbind
    nonAffine(quadVertices);

    //Framebuffer for Video Target - Video Texture
    glGenFramebuffers(1, &video_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, video_framebuffer);
    glGenTextures(1, &video_textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, video_textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, video_textureColorbuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: VIDEO Framebuffer #" << video_framebuffer << "is not complete!" << std::endl;
    glEnable(GL_MULTISAMPLE);

    mpv_fbo.fbo = static_cast<int>(video_framebuffer);
    mpv_fbo.internal_format = 0;
    mpv_fbo.w = window_width;
    mpv_fbo.h = window_height;

    int flip_y{0};
    params_fbo[0] = {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo};
    params_fbo[1] = {MPV_RENDER_PARAM_FLIP_Y, &flip_y};
    params_fbo[2] = {MPV_RENDER_PARAM_INVALID, nullptr};

    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (animation)
        {
            quadVertices[0] = sin(currentFrame) / 4. - 0.5;
            quadVertices[1] = cos(currentFrame) / 4. - 0.5;
            quadVertices[9] = sin(currentFrame * 0.2) / 4. + 0.5;
            quadVertices[10] = cos(currentFrame * 0.9) / 4. - 0.5;
            quadVertices[18] = sin(currentFrame * 0.3) / 4. + 0.5;
            quadVertices[19] = cos(currentFrame * 1.1) / 4. + 0.5;
            quadVertices[27] = sin(currentFrame * 0.6) / 4. - 0.5;
            quadVertices[28] = cos(currentFrame * 1.4) / 4. + 0.5;
        }

        processGLFWInput(window);
        // -----
        nonAffine(quadVertices);
        if (wakeup)
        {
            if ((mpv_render_context_update(mpv_ctx) & MPV_RENDER_UPDATE_FRAME))
            {
                mpv_render_context_render(mpv_ctx, params_fbo);
                glViewport(0, 0, window_width, window_height);
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        if (showfx)
        {
            glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        }
        else
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, video_textureColorbuffer);

        glEnable(GL_PROGRAM_POINT_SIZE);
        glEnable(GL_MULTISAMPLE);

        if (showfx)
        {
            fxShader->use();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            pointShader.use();
            glDrawElements(GL_POINTS, 6, GL_UNSIGNED_INT, 0);
        }
        else
        {
            screenShader->setFloat("vignette", vignette);
            screenShader->use();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        // -----
        if (wakeup)
        {
            mpv_render_context_report_swap(mpv_ctx);
            wakeup = 0;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        usleep(1000);
    }
    mpv_render_context_free(mpv_ctx);
    mpv_detach_destroy(mpv);

    glfwTerminate();
    return 0;
}

void processGLFWInput(GLFWwindow *window)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
    {

        if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, 0.1, quadVertices[0], quadVertices[1]) || activecorner == 0)
        {
            quadVertices[0] = x / window_width * 2 - 1;
            quadVertices[1] = 1 - y / window_height * 2;
            activecorner = 0;
        }
        if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, 0.1, quadVertices[9], quadVertices[10]) || activecorner == 1)
        {
            quadVertices[9] = x / window_width * 2 - 1;
            quadVertices[10] = 1 - y / window_height * 2;
            activecorner = 1;
        }

        if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, 0.1, quadVertices[18], quadVertices[19]) || activecorner == 2)
        {
            quadVertices[18] = x / window_width * 2 - 1;
            quadVertices[19] = 1 - y / window_height * 2;
            activecorner = 2;
        }

        if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, 0.1, quadVertices[27], quadVertices[28]) || activecorner == 3)
        {
            quadVertices[27] = x / window_width * 2 - 1;
            quadVertices[28] = 1 - y / window_height * 2;
            activecorner = 3;
        }
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
        activecorner = -1;

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_FALSE);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        animation = !animation;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        memcpy(quadVertices, quadVertices_orig, sizeof(quadVertices));
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        showfx = !showfx;
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        vignette += 0.1;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
    {
        if (vignette > 0)
            vignette -= 0.1;
    }
}

// Returns the address of the specified function (name) for the given context (ctx)
static void *get_proc_address(void *ctx, const char *name)
{
    glfwGetCurrentContext();
    return reinterpret_cast<void *>(glfwGetProcAddress(name));
}

static void on_mpv_render_update(void *ctx)
{
    wakeup = 1;
}

static void on_mpv_events(void *ctx)
{
    // std::cout << "INFO::" << __func__ << std::endl;
}

int nonAffine(float *vertex)
{

    float ax = vertex[18] - vertex[0];
    float ay = vertex[19] - vertex[1];
    float bx = vertex[27] - vertex[9];
    float by = vertex[28] - vertex[10];

    float cross = ax * by - ay * bx;

    if (cross != 0)
    {
        float cy = vertex[1] - vertex[10];
        float cx = vertex[0] - vertex[9];
        float s = (ax * cy - ay * cx) / cross;
        if (s > 0 && s < 1)
        {
            float t = (bx * cy - by * cx) / cross;
            if (t > 0 && t < 1)
            {
                float u0 = 0;
                float v0 = 0;
                float u2 = 1;
                float v2 = 1;

                float q0 = 1 / (1 - t);
                float q1 = 1 / (1 - s);
                float q2 = 1 / t;
                float q3 = 1 / s;

                vertex[3] = u0 * q0;
                vertex[4] = v2 * q0;
                vertex[5] = q0;

                vertex[12] = u2 * q1;
                vertex[13] = v2 * q1;
                vertex[14] = q1;

                vertex[21] = u2 * q2;
                vertex[22] = v0 * q2;
                vertex[23] = q2;

                vertex[30] = u0 * q3;
                vertex[31] = v0 * q3;
                vertex[32] = q3;
            }
            else
            {
                return 0;
                std::cout << "T not in range:" << t << std::endl;
            }
        }
        else
        {
            return 0;
            std::cout << "S not in range:" << s << std::endl;
        }
    }
    else
    {
        return 0;
        std::cout << "CROSS is ZERO:" << cross << std::endl;
    }
    return 1;
}

float inline inCircleN(float x, float y, float r, float x0, float y0)
{
    float dx = abs(x - x0);
    float dy = abs(y - y0);
    return (dx * dx + dy * dy <= r * r);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    window_height = height;
    window_width = width;
    glBindTexture(GL_TEXTURE_2D, video_textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    mpv_fbo.fbo = static_cast<int>(video_framebuffer);
    mpv_fbo.internal_format = 0;
    mpv_fbo.w = window_width;
    mpv_fbo.h = window_height;

    int flip_y{0};
    params_fbo[0] = {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo};
    params_fbo[1] = {MPV_RENDER_PARAM_FLIP_Y, &flip_y};
    params_fbo[2] = {MPV_RENDER_PARAM_INVALID, nullptr};
}
