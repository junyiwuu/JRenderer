#include "window.hpp"



JWindow::JWindow(uint32_t width, uint32_t height, const char* title):width(width),height(height), title(title){
    initWindow();
}

JWindow::~JWindow(){
    glfwDestroyWindow(window);
    glfwTerminate();
}




void JWindow::initWindow(){
    glfwInit();  // must call it first, otherwise glfwCreateWindow will return null

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void JWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height){
    auto app = reinterpret_cast<JWindow*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}























