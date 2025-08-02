#include "global.hpp"
#include "window.hpp"
#include "../Renderers/interactiveSystem.hpp"
#include <GLFW/glfw3.h>
#include "imgui.h"

namespace tools
{
    bool errorModeSilent = false;
    std::string resourcePath = "";

    std::string errorString(VkResult errorCode)
    {
        switch (errorCode)
        {
#define STR(r) case VK_ ##r: return #r
            STR(NOT_READY);
            STR(TIMEOUT);
            STR(EVENT_SET);
            STR(EVENT_RESET);
            STR(INCOMPLETE);
            STR(ERROR_OUT_OF_HOST_MEMORY);
            STR(ERROR_OUT_OF_DEVICE_MEMORY);
            STR(ERROR_INITIALIZATION_FAILED);
            STR(ERROR_DEVICE_LOST);
            STR(ERROR_MEMORY_MAP_FAILED);
            STR(ERROR_LAYER_NOT_PRESENT);
            STR(ERROR_EXTENSION_NOT_PRESENT);
            STR(ERROR_FEATURE_NOT_PRESENT);
            STR(ERROR_INCOMPATIBLE_DRIVER);
            STR(ERROR_TOO_MANY_OBJECTS);
            STR(ERROR_FORMAT_NOT_SUPPORTED);
            STR(ERROR_SURFACE_LOST_KHR);
            STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
            STR(SUBOPTIMAL_KHR);
            STR(ERROR_OUT_OF_DATE_KHR);
            STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
            STR(ERROR_VALIDATION_FAILED_EXT);
            STR(ERROR_INVALID_SHADER_NV);
            STR(ERROR_INCOMPATIBLE_SHADER_BINARY_EXT);
#undef STR
        default:
            return "UNKNOWN_ERROR";
        }
    }
}







// AppContext implementation
void AppContext::registerAllCallbacks(GLFWwindow* window) {
    // Set this AppContext as the user pointer
    glfwSetWindowUserPointer(window, this);
    
    // Register all callbacks
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetKeyCallback(window, keyCallback);
}

void AppContext::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto* context = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (context && context->window_) {
        // Call the window's resize handling
        context->window_->setFramebufferResized(true);
    }
}

void AppContext::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* context = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (!context || !context->interactiveSystem_) return;
    
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    // Handle ImGui input first
    const ImGuiMouseButton_ imguiButton = (button == GLFW_MOUSE_BUTTON_LEFT)
            ? ImGuiMouseButton_Left
            : (button == GLFW_MOUSE_BUTTON_RIGHT ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);

    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[imguiButton] = action == GLFW_PRESS;

    // Only handle camera input if ImGui doesn't want the mouse
    if (!io.WantCaptureMouse) {
        context->interactiveSystem_->handleMouseButton(button, action, xpos, ypos);
    }
}

void AppContext::cursorPosCallback(GLFWwindow* window, double x, double y) {
    auto* context = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (!context || !context->interactiveSystem_) return;
    
    // Update ImGui mouse position
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2((float)x, (float)y);
    
    // Only handle camera input if ImGui doesn't want the mouse
    if (!io.WantCaptureMouse) {
        context->interactiveSystem_->handleCursorPos(x, y, window);
    }
}

void AppContext::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* context = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    if (!context || !context->interactiveSystem_) return;
    
    // Delegate to InteractiveSystem
    context->interactiveSystem_->handleKeyboard(key, scancode, action, mods);
}































































