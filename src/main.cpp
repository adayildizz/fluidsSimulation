#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "Angel.h"
#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

GLuint mainShaderProgram;
GLuint presentProgram;
GLuint presentVAO;
GLuint vbo, vao;
GLuint fbo;
GLuint texture;
GLuint depthTexture;

// about points 
const int N = 2000;
std::vector<float> pos(N*3);
std::vector<float> vel(N*3);




// Error callback for GLFW
void errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Key callback for closing window
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void createFBO(int w, int h){
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);


    // depth texture for now
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT,nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    GLenum bufs[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, bufs);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO incomplete!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void init(){
    // absolute path since we are in D
    mainShaderProgram = InitShader("D:/fluidmechanicssimulation/shaders/vertex.glsl",
                               "D:/fluidmechanicssimulation/shaders/fragment.glsl");
    presentProgram = InitShader("D:/fluidmechanicssimulation/shaders/presentv.glsl",
                               "D:/fluidmechanicssimulation/shaders/presentf.glsl");
        glGenVertexArrays(1, &presentVAO);


    std::mt19937 rng(42);
    std::uniform_real_distribution<float> pinit(-0.6f, 0.6f);
    std::uniform_real_distribution<float> vinit(-0.5f, 0.5f);

    for (int i = 0; i < N; ++i) {
        pos[3*i+0] = pinit(rng);
        pos[3*i+1] = pinit(rng) + 0.4f;      // a bit higher so they fall
        pos[3*i+2] = pinit(rng);
        vel[3*i+0] = vinit(rng) * 0.6f;
        vel[3*i+1] = vinit(rng) * 0.6f;
        vel[3*i+2] = vinit(rng) * 0.6f;

    }

    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, pos.size()* sizeof(float), pos.data(), GL_DYNAMIC_DRAW);

    // 3 is the dim we are working on
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_PROGRAM_POINT_SIZE);

}

int main() {
    // Set error callback before init
    glfwSetErrorCallback(errorCallback);

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

    // Initialize GLEW
    glewExperimental = GL_TRUE;  // This is needed for core profile
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Clear any GLEW initialization errors
    glGetError();

    // Print OpenGL version
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;

    // Set viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    init();

    // Timing 
    double prevTime = glfwGetTime();
    // margins kept for the round sprites are not clipped
    const float xmin = -0.98f, xmax = 0.98f;
    const float ymin = -0.98f, ymax = 0.98f;
    const float zmin = -0.98f, zmax = 0.98f;
    

    const float gravity = -1.5f;
    const float bounce = 0.6f;
    const float drag = 0.999f;

    GLuint uPointSize = glGetUniformLocation(mainShaderProgram, "uPointSize");
    GLuint uMVP = glGetUniformLocation(mainShaderProgram, "uMVP");

    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    createFBO(fbw, fbh);
    // Main loop
    while (!glfwWindowShouldClose(window)) {

         // --- dt ---
        double now = glfwGetTime();
        float dt = float(now - prevTime);
        prevTime = now;

        // --- CPU integrate ---
        for (int i = 0; i < N; ++i) {
            float& x  = pos[3*i+0];
            float& y  = pos[3*i+1];
            float& z  = pos[3*i+2];
            float& vx = vel[3*i+0];
            float& vy = vel[3*i+1];
            float& vz = vel[3*i+2];

            vy += gravity * dt;
            // drag simulates the air resistance
            vx *= drag; vy *= drag; vz *= drag;

            x += vx * dt;
            y += vy * dt;
            z += vz * dt;

            // collide with bounds (simple reflection with damping)
            if (x < xmin) { x = xmin + (xmin - x); vx = -vx * bounce; }
            if (x > xmax) { x = xmax - (x - xmax); vx = -vx * bounce; }
            if (y < ymin) { y = ymin + (ymin - y); vy = -vy * bounce; }
            if (y > ymax) { y = ymax - (y - ymax); vy = -vy * bounce; }
            if (z < zmin) { z = zmin + (zmin - z); vz = -vz * bounce; }
            if (z > zmax) { z = zmax - (z - zmax); vz = -vz * bounce; }
        }

        mat4 model = mat4(1.0f);
        mat4 view = LookAt(
            vec4(0.0f, 0.5f, 3.0f, 1.0f),
            vec4(0.0f,0.0f,0.0f, 1.0f),
            vec4(0.0f, 1.0f,0.0f, 0.0f));

        int fbw, fbh;
        glfwGetFramebufferSize(window, &fbw, &fbh);
        float aspect = (fbw > 0 && fbh > 0) ? float(fbw)/float(fbh) : 1.0f;

        mat4 proj = Perspective(45.0, aspect, 0.1f, 100.0f);
        mat4 mvp = proj * view * model;



         // --- stream new positions to GPU ---



        // glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // glBufferSubData(GL_ARRAY_BUFFER, 0, pos.size()*sizeof(float), pos.data());

        // // --- draw ---
        // glEnable(GL_DEPTH_TEST);
        // glDepthFunc(GL_LESS);

        // glClearColor(0.07f, 0.07f, 0.09f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // glUseProgram(mainShaderProgram);
        // glUniform1f(uPointSize, 6.0f); 
        // glUniformMatrix4fv(uMVP, 1, GL_TRUE, mvp);
        // glBindVertexArray(vao);
        // glDrawArrays(GL_POINTS, 0, N);

        // 3.1 — Render particles into the offscreen FBO
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, fbw, fbh);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glUseProgram(mainShaderProgram);
        glUniform1f(uPointSize, 6.0f);
        glUniformMatrix4fv(uMVP, 1, GL_TRUE, &mvp[0][0]); // or GL_FALSE if that’s what worked for you

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, pos.size()*sizeof(float), pos.data());
        glDrawArrays(GL_POINTS, 0, N);

        // 3.2 — Present the offscreen color to the window
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);   // back to window size
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(presentProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(presentProgram, "uTex"), 0);

        glBindVertexArray(presentVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);



        glfwSwapBuffers(window);
        glfwPollEvents();

        
    }
    // Cleanup
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(mainShaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "Program exited successfully" << std::endl;
    return 0;
}