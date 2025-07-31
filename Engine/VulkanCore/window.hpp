#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <map>
#include <functional>


class JWindow{


public:
    JWindow(uint32_t width, uint32_t height, const char* title);
    ~JWindow();

    JWindow(const JWindow&) = delete;
    JWindow &operator=(const JWindow&) = delete;

    bool shouldClose() {return glfwWindowShouldClose(window);}
    GLFWwindow* getGLFWwindow() const {return window;}
    bool ifFramebufferResized() {return framebufferResized; }
    void resetWindowResizedFlag() {framebufferResized = false; }

    
private:


    void initWindow();

    GLFWwindow* window;

    uint32_t width;
    uint32_t height;
    const char* title;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    bool framebufferResized = false;
};