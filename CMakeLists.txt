cmake_minimum_required(VERSION 3.15)
project(OpenGLProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "C:/Users/yildiz/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Find required packages
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# Create executable
add_executable(${PROJECT_NAME} src/main.cpp)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw
    GLEW::GLEW
    glm::glm
    OpenGL::GL
)

# Copy DLLs to build directory (for Windows)
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:glfw> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:GLEW::GLEW> $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()