// GNU GPLv3
// Copyright (c) 2020,2021 v0idv0id - Martin Willner - lvslinux@gmail.com

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

    if (argc == 10)
    {
        GLFWmonitor *primary = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(primary);
        window_width = mode->width;
        window_height = mode->height;
        window = glfwCreateWindow(window_width, window_height, "3dmpv", glfwGetPrimaryMonitor(), NULL); // for borderless fullscreen windows
    }
    else
    {
        window = glfwCreateWindow(window_width, window_height, "3dmpv - 1", NULL, NULL);
        //   window2 = glfwCreateWindow(window_width, window_height, "3dmpv - 2", NULL, NULL);
    }
    if (window == NULL)
    {
        std::cout << "ERROR::GLFW::Failed to create window" << std::endl;
        return -1;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ERROR::GLAD::Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // MPV initialization and configuration
    for (int i = 0; i < MAXINSTANCES; i++)
    {
        mpv[i] = mpv_create();
        activecorner[i] = -1;
        mpv_request_log_messages(mpv[i], "debug");
        mpv_set_option_string(mpv[i], "terminal", "yes");
        mpv_set_option_string(mpv[i], "config", "no"); // do not load anything from ~/.config/mpv/
        mpv_set_option_string(mpv[i], "gpu-api", "opengl");
        mpv_set_option_string(mpv[i], "vd-lavc-dr", "yes");
        mpv_set_option_string(mpv[i], "hwdec", "auto");
        mpv_set_option_string(mpv[i], "vo", "libmpv");
        mpv_set_option_string(mpv[i], "loop", "");
        mpv_set_option_string(mpv[i], "load-unsafe-playlists", "");
        mpv_set_option_string(mpv[i], "load-scripts", "no");
        mpv_set_option_string(mpv[i], "interpolation", "yes");
        mpv_set_option_string(mpv[i], "video-sync", "display-resample");
        mpv_set_option_string(mpv[i], "video-timing-offset", "0"); //fixes FPS locked to video FPS

        // mpv_set_option_string(mpv, "scripts-add", "./webui-page/xwebui.lua");
        // mpv_set_option_string(mpv, "scripts-append", "./webui-page/xwebui.lua");
        // mpv_set_option_string(mpv, "script", "xwebui.lua");

        //  mpv_set_option_string(mpv, "msg-level", "all=debug");

        if (mpv_initialize(mpv[i]) < MPV_ERROR_SUCCESS)
        {
            std::cout << "ERROR::MPV::Failed to initialize mpv instance" << i << std::endl;
            return -1;
        }

        mpv_opengl_init_params opengl_init_params{
            get_proc_address,
            nullptr,
            nullptr};

        int adv = 1;

        mpv_render_param render_param[]{
            {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
            {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &opengl_init_params},
            {MPV_RENDER_PARAM_ADVANCED_CONTROL, &adv},
            {MPV_RENDER_PARAM_INVALID, nullptr},
        };

        if (mpv_render_context_create(&mpv_ctx[i], mpv[i], render_param) < MPV_ERROR_SUCCESS)
        {
            std::cout << "ERROR::MPV::Failed to create MPV render context instance" << i << std::endl;
            return -1;
        }

        mpv_set_wakeup_callback(mpv[i], on_mpv_events, NULL);
        mpv_render_context_set_update_callback(mpv_ctx[i], on_mpv_render_update, NULL);

        memcpy(quadVertices[i], quadVertices_orig, sizeof(quadVertices[0]));
    }
    // SHADER creation

    Shader *screenShader = new Shader("shaders/screen_vs.glsl", "shaders/screen_fs.glsl");
    Shader *fxShader = new Shader("shaders/fx_vs.glsl", "shaders/fx_fs.glsl");
    Shader pointShader("shaders/point_vs.glsl", "shaders/point_fs.glsl");

    glEnable(GL_DEPTH);
    glEnable(GL_MULTISAMPLE);

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
    nonAffine(quadVertices[0]);

    for (int i = 0; i < MAXINSTANCES; i++)
    {
        //Framebuffer for Video Target - Video Texture
        glGenFramebuffers(1, &video_framebuffer[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, video_framebuffer[i]);
        glGenTextures(1, &video_textureColorbuffer[i]);
        glBindTexture(GL_TEXTURE_2D, video_textureColorbuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, video_textureColorbuffer[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: VIDEO Framebuffer #" << video_framebuffer[i] << "is not complete!" << std::endl;

        mpv_fbo[i].fbo = static_cast<int>(video_framebuffer[i]);
        mpv_fbo[i].internal_format = GL_RGB;
        mpv_fbo[i].w = window_width;
        mpv_fbo[i].h = window_height;

        int flip_y{0};
        params_fbo[i][0] = {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo[i]};
        params_fbo[i][1] = {MPV_RENDER_PARAM_FLIP_Y, &flip_y};
        params_fbo[i][2] = {MPV_RENDER_PARAM_INVALID, nullptr};
    }
    glEnable(GL_MULTISAMPLE);
    glDepthRange(10, -10);

    for (int i = 0; i < argc - 1; i++)
    {
        const char *cmd[] = {"loadfile", argv[i + 1], NULL};
        std::cout << "Loadfile: " << argv[i + 1] << std::endl;

        mpv_command(mpv[i], cmd);
        // const char *s1[]={"add", "volume","-100",NULL};
        // mpv_command(mpv[i],s1);
        active_instances++;
    }

    while (!glfwWindowShouldClose(window))
    {
        processGLFWInput(window);
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        for (int i = 0; i < active_instances; i++)
        {
            if (animation)
            {
                quadVertices[i][0] = sin(currentFrame + i) / 4. - 0.8;
                quadVertices[i][1] = cos(currentFrame + i) / 4. - 0.8;
                quadVertices[i][2] = cos(currentFrame + i);

                quadVertices[i][9] = sin(currentFrame * 0.2 + i) / 4. + 0.5;
                quadVertices[i][10] = cos(currentFrame * 0.9 + i) / 4. - 0.5;
                quadVertices[i][11] = cos(currentFrame + i);

                quadVertices[i][18] = sin(currentFrame * 0.3 + i) / 4. + 0.5;
                quadVertices[i][19] = cos(currentFrame * 1.1 + i) / 4. + 0.5;
                quadVertices[i][20] = cos(currentFrame + i);

                quadVertices[i][27] = sin(currentFrame * 0.6 + i) / 4. - 0.5;
                quadVertices[i][28] = cos(currentFrame * 1.4 + i) / 4. + 0.5;
                quadVertices[i][29] = cos(currentFrame + i);
            }
            nonAffine(quadVertices[i]);

            if ((mpv_render_context_update(mpv_ctx[i]) & MPV_RENDER_UPDATE_FRAME))
            {
                mpv_render_context_render(mpv_ctx[i], params_fbo[i]);
            }
        }

        glViewport(0, 0, window_width, window_height);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_DEPTH_TEST);

        for (int i = 0; i < active_instances; i++)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBindTexture(GL_TEXTURE_2D, video_textureColorbuffer[i]);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices[i]), &quadVertices[i], GL_STATIC_DRAW);
            glEnable(GL_PROGRAM_POINT_SIZE);
            glEnable(GL_MULTISAMPLE);
            if (showfx)
            {
                fxShader->use();
                fxShader->setFloat("trans_alpha", trans_alpha);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                pointShader.use();
                glDrawElements(GL_POINTS, 6, GL_UNSIGNED_INT, 0);
            }
            else
            {
                screenShader->use();
                screenShader->setFloat("vignette", vignette);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        if (wakeup)
        {
            for (int i = 0; i < active_instances; i++)
                mpv_render_context_report_swap(mpv_ctx[i]);

            wakeup = 0;
        }
        glfwSwapBuffers(window);

        glfwPollEvents();
        usleep(1000);
    }

    for (int i = 0; i < active_instances; i++)
    {
        mpv_render_context_free(mpv_ctx[i]);
        mpv_detach_destroy(mpv[i]);
    }
    glfwTerminate();
    return 0;
}

void processGLFWInput(GLFWwindow *window)
{
    double x, y;
    double radius = 0.1;
    return;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
    {
        for (int i = 0; i < active_instances; i++)
        {
            glfwGetCursorPos(window, &x, &y);
            if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, radius, quadVertices[i][0], quadVertices[i][1]) || activecorner[i] == 0)
            {
                quadVertices[i][0] = x / window_width * 2 - 1;
                quadVertices[i][1] = 1 - y / window_height * 2;
                activecorner[i] = 0;
            }
            if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, radius, quadVertices[i][9], quadVertices[i][10]) || activecorner[i] == 1)
            {
                quadVertices[i][9] = x / window_width * 2 - 1;
                quadVertices[i][10] = 1 - y / window_height * 2;
                activecorner[i] = 1;
            }

            if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, radius, quadVertices[i][18], quadVertices[i][19]) || activecorner[i] == 2)
            {
                quadVertices[i][18] = x / window_width * 2 - 1;
                quadVertices[i][19] = 1 - y / window_height * 2;
                activecorner[i] = 2;
            }

            if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, radius, quadVertices[i][27], quadVertices[i][28]) || activecorner[i] == 3)
            {
                quadVertices[i][27] = x / window_width * 2 - 1;
                quadVertices[i][28] = 1 - y / window_height * 2;
                activecorner[i] = 3;
            }
        }
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
        for (int i = 0; i < active_instances; i++)
            activecorner[i] = -1;
}

// Returns the address of the specified function (name) for the given context (ctx)
static void *get_proc_address(void *ctx, const char *name)
{
    glfwGetCurrentContext();
    return reinterpret_cast<void *>(glfwGetProcAddress(name));
}

static void on_mpv_render_update(void *ctx)
{
#ifdef DEBUG
    std::cout << "DEBUG INFO::" << __func__ << std::endl;
#endif
    wakeup = 1;
}

static void on_mpv_events(void *ctx)
{
#ifdef DEBUG
    std::cout << "DEBUG INFO::" << __func__ << std::endl;
#endif
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
    float dx = fabs(x - x0);
    float dy = fabs(y - y0);
    return (dx * dx + dy * dy <= r * r);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
#ifdef DEBUG
    std::cout << "DEBUG INFO::" << __func__ << std::endl;
#endif
    window_height = height;
    window_width = width;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_FALSE);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
        animation = !animation;
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        for (int i = 0; i < MAXINSTANCES; i++)
            memcpy(quadVertices[i], quadVertices_orig, sizeof(quadVertices_orig));
    }
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        showfx = !showfx;
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
        vignette += 0.2;
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        if (vignette > 0)
            vignette -= 0.2;
    }
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {

        trans_alpha -= 0.2;
        if (trans_alpha < 0)
            trans_alpha = 0;
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        trans_alpha += 0.2;
        if (trans_alpha > 1.0)
            trans_alpha = 1.0;
    }
}

void cursor_position_callback(GLFWwindow *window, double x, double y)
{
    float radius=0.05;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
    {
        for (int i = 0; i < active_instances; i++)
        {
            if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, radius, quadVertices[i][0], quadVertices[i][1]) || activecorner[i] == 0)
            {
                quadVertices[i][0] = x / window_width * 2 - 1;
                quadVertices[i][1] = 1 - y / window_height * 2;
                activecorner[i] = 0;
            }
            if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, radius, quadVertices[i][9], quadVertices[i][10]) || activecorner[i] == 1)
            {
                quadVertices[i][9] = x / window_width * 2 - 1;
                quadVertices[i][10] = 1 - y / window_height * 2;
                activecorner[i] = 1;
            }

            if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, radius, quadVertices[i][18], quadVertices[i][19]) || activecorner[i] == 2)
            {
                quadVertices[i][18] = x / window_width * 2 - 1;
                quadVertices[i][19] = 1 - y / window_height * 2;
                activecorner[i] = 2;
            }

            if (inCircleN(x / window_width * 2 - 1, 1 - y / window_height * 2, radius, quadVertices[i][27], quadVertices[i][28]) || activecorner[i] == 3)
            {
                quadVertices[i][27] = x / window_width * 2 - 1;
                quadVertices[i][28] = 1 - y / window_height * 2;
                activecorner[i] = 3;
            }
        }
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
        for (int i = 0; i < active_instances; i++)
            activecorner[i] = -1;
}