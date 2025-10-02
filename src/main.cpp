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
GLuint vbo, vao;

// about points 
const int N = 20;
std::vector<float> pos(N*2);
std::vector<float> vel(N*2);




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


void init(){
    float initPos[2] = {0.0f, 0.0f};

    // absolute path since we are in D
    mainShaderProgram = InitShader("D:/fluidmechanicssimulation/shaders/vertex.glsl",
                               "D:/fluidmechanicssimulation/shaders/fragment.glsl");


    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> pinit(-0.6f, 0.6f);
    std::uniform_real_distribution<float> vinit(-0.5f, 0.5f);

    for (int i = 0; i < N; ++i) {
        pos[2*i+0] = pinit(rng);
        pos[2*i+1] = pinit(rng) + 0.4f;      // a bit higher so they fall
        vel[2*i+0] = vinit(rng) * 0.6f;
        vel[2*i+1] = vinit(rng) * 0.6f;
    }

    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, pos.size()* sizeof(float), pos.data(), GL_DYNAMIC_DRAW);


    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
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

    const float gravity = -1.5f;
    const float bounce = 0.6f;
    const float drag = 0.999f;

GLint uPointSize = glGetUniformLocation(mainShaderProgram, "uPointSize");
    // Main loop
    while (!glfwWindowShouldClose(window)) {

         // --- dt ---
        double now = glfwGetTime();
        float dt = float(now - prevTime);
        prevTime = now;

        // --- CPU integrate ---
        for (int i = 0; i < N; ++i) {
            float& x  = pos[2*i+0];
            float& y  = pos[2*i+1];
            float& vx = vel[2*i+0];
            float& vy = vel[2*i+1];

            vy += gravity * dt;
            vx *= drag; vy *= drag;

            x += vx * dt;
            y += vy * dt;

            // collide with bounds (simple reflection with damping)
            if (x < xmin) { x = xmin + (xmin - x); vx = -vx * bounce; }
            if (x > xmax) { x = xmax - (x - xmax); vx = -vx * bounce; }
            if (y < ymin) { y = ymin + (ymin - y); vy = -vy * bounce; }
            if (y > ymax) { y = ymax - (y - ymax); vy = -vy * bounce; }
        }



         // --- stream new positions to GPU ---
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, pos.size()*sizeof(float), pos.data());

        // --- draw ---
        glClearColor(0.07f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(mainShaderProgram);
        glUniform1f(uPointSize, 6.0f); // try 3..12
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, N);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // // Clear the screen
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // // rendering code starts

        // glUseProgram(mainShaderProgram);
        // GLint loc = glGetUniformLocation(mainShaderProgram, "uPointSize");
        // glUniform1f(loc, 24.0f);

        // glBindVertexArray(vao);
        // glDrawArrays(GL_POINTS, 0,1);
        // // rendering code ends

        // // Swap buffers and poll events
        // glfwSwapBuffers(window);
        // glfwPollEvents();
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