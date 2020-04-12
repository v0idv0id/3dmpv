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
    glfwWindowHint(GLFW_SAMPLES, 8);

    // GLFWmonitor *primary = glfwGetPrimaryMonitor();
    // const GLFWvidmode *mode = glfwGetVideoMode(primary);
     window_width = 800; //mode->width;
     window_height = 600; //mode->height;
    fbo_width = 800; //mode->width;
    fbo_height = 600; //mode->height;

    if ((window = glfwCreateWindow(window_width, window_height, "MPVideoCube", NULL, NULL)) == NULL)
    {
        std::cout << "ERROR::GLFW::Failed to create window" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);
    // glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ERROR::GLAD::Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH);
    glEnable(GL_MULTISAMPLE);

    // MPV initialization and configuration
    mpv = mpv_create();
    if (mpv_initialize(mpv) < MPV_ERROR_SUCCESS)
    {
        std::cout << "ERROR::MPV::Failed to initialize mpv" << std::endl;
        return -1;
    }

    mpv_opengl_init_params opengl_init_params{
        get_proc_address,
        nullptr,
        nullptr};

    int adv{1};

    mpv_render_param render_param[]{
        {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
        {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &opengl_init_params},
        {MPV_RENDER_PARAM_ADVANCED_CONTROL, &adv},
        {MPV_RENDER_PARAM_INVALID, nullptr},
    };

    if (mpv_render_context_create(&mpv_ctx, mpv, render_param) < MPV_ERROR_SUCCESS)
    {
        std::cout << "ERROR::MPV::Failed to create MPV render context" << std::endl;
        return -1;
    }

    mpv_set_wakeup_callback(mpv, on_mpv_events, NULL);
    mpv_render_context_set_update_callback(mpv_ctx, on_mpv_render_update, NULL);

    // const char *cmd[] = {"loadfile", argv[1], NULL};
    const char *cmd[] = {"loadlist", argv[1], NULL};
    mpv_command(mpv, cmd);
    mpv_set_option_string(mpv, "loop", "");
    mpv_set_option_string(mpv, "load-unsafe-playlists", "");
    mpv_set_option_string(mpv, "gpu-api", "opengl");
    mpv_set_option_string(mpv, "hwdec", "auto");
    mpv_set_option_string(mpv, "terminal", "yes");

    // SHADER creation

    Shader cubeShader("shaders/cube_vs.glsl", "shaders/cube_fs.glsl");
    Shader *screenShader = new Shader("shaders/screen_vs.glsl", "shaders/screen_fs.glsl");
    Shader pointShader("shaders/point_vs.glsl", "shaders/point_fs.glsl");
    Shader fxShader("shaders/fx_vs.glsl", "shaders/fx_fs.glsl");

    memcpy(quadVertices, quadVertices_orig, sizeof(quadVertices));

    // CUBE Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0); // coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float))); //texture

    // SCREEN QUAD
    unsigned int quadVAO, quadVBO, quadEBO;
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
    glBindVertexArray(0); // unbind
    nonAffine(quadVertices);

    //Framebuffer for Video Target - Video Texture
    glGenFramebuffers(1, &video_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, video_framebuffer);
    // create a color attachment texture
    glGenTextures(1, &video_textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, video_textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbo_width, fbo_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, video_textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    // glGenRenderbuffers(1, &video_rbo);
    // glBindRenderbuffer(GL_RENDERBUFFER, video_rbo);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbo_width, fbo_height);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, video_rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: VIDEO Framebuffer #" << video_framebuffer << "is not complete!" << std::endl;

    //Framebuffer for Screen Target - Main Screen

    glGenFramebuffers(1, &screen_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, screen_framebuffer);
    // create a color attachment texture
    glGenTextures(1, &screen_textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, screen_textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screen_textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &screen_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, screen_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_width, window_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, screen_rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: SCREEN Framebuffer #" << screen_framebuffer << "is not complete!" << std::endl;

    mpv_fbo.fbo = static_cast<int>(video_framebuffer);
    mpv_fbo.internal_format = 0;
    mpv_fbo.w = fbo_width;
    mpv_fbo.h = fbo_height;

    int flip_y{1};

    mpv_render_param params_fbo[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &mpv_fbo},
        {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
        {MPV_RENDER_PARAM_INVALID, nullptr}};

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (0)
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

        mpv_render_context_render(mpv_ctx, params_fbo); // this "renders" to the video_framebuffer "linked by ID" in the params_fbo - BLOCKING

        // **************** RENDER TO THE SCREEN FBO
        glBindFramebuffer(GL_FRAMEBUFFER, screen_framebuffer); // <-- BIND THE SCREEN FBO
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f); // RED BACKGROUND
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        cubeShader.use(); // <-- The CUBE SHADER
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        // glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)window_width / (float)window_height, 0.1f, 100.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)window_width / (float)window_height, 0.1f, 100.0f);

        cubeShader.setMat4("view", view);
        cubeShader.setMat4("projection", projection);
        model = glm::rotate(model, currentFrame, glm::vec3(sin(currentFrame / 10.), cos(currentFrame / 10.), sin(currentFrame / 10.) * cos(currentFrame / 10.)));
        // model=glm::scale(model,glm::vec3(2.,2.,2.));
        glBindVertexArray(cubeVAO); // <-- The CUBE
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, video_textureColorbuffer); // <-- VIDEO Colorbuffer IS THE TEXTURE

        cubeShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36); // CUBE 1

        // model = glm::rotate(model, currentFrame, glm::vec3(sin(currentFrame / 3.), cos(currentFrame / 7.), sin(currentFrame / 11.) * cos(currentFrame / 2.)));
        // model = glm::translate(model, glm::vec3(1 * sin(currentFrame / 2.0), sin(currentFrame / 13.0), 1 * sin(currentFrame / 53.0)));
        // cubeShader.setMat4("model", model);
        // glDrawArrays(GL_TRIANGLES, 0, 36); // CUBE 2

        // **************** RENDER TO THE MAIN FBO.
        // We use the QUAD with the SCREEN FBO as texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // <-- BIND THE DEFAULT FBO
        glDisable(GL_DEPTH_TEST);             // NO DEPTH TEST!

        glEnable(GL_MULTISAMPLE);
        // glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(quadVAO);

        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screen_textureColorbuffer); // <-- SCREEN Colorbuffer IS THE TEXTURE
        glEnable(GL_PROGRAM_POINT_SIZE);

        screenShader->use();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        pointShader.use();
        glDrawElements(GL_POINTS, 6, GL_UNSIGNED_INT, 0);

        fxShader.use();

        // -----
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        mpv_command_string(mpv, "playlist-next");
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        mpv_command_string(mpv, "playlist-prev");
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        mpv_command_string(mpv, "display-fps");
}

// Returns the address of the specified function (name) for the given context (ctx)
static void *get_proc_address(void *ctx, const char *name)
{
    glfwGetCurrentContext();
    return reinterpret_cast<void *>(glfwGetProcAddress(name));
}

static void on_mpv_render_update(void *ctx)
{
    // std::cout << "INFO::" << __func__ << std::endl;
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
    glViewport(0, 0, width, height);


    // fbo_width = window_width;
    // fbo_height = window_height;
    // mpv_fbo.w = fbo_width;
    // mpv_fbo.h = fbo_height;
}
