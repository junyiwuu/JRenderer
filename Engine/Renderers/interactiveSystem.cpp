#include "interactiveSystem.hpp"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "../VulkanCore/window.hpp"
#include "../VulkanCore/device.hpp"
#include "../VulkanCore/swapchain.hpp"




InteractiveSystem::InteractiveSystem(JWindow& window, JDevice& device, const JSwapchain& swapchain):
    window_app(window),
    device_app(device),
    swapchain_app(swapchain),


    camera_Arcball_positioner{
        window_app,
        glm::vec3(0.0f, 0.0f, -2.0f), 
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f),
        Scene::DragMode::None},
    camera_Arcball{camera_Arcball_positioner}

{
    imgui_obj = std::make_unique<JImGui>(device_app, swapchain_app, window_app.getGLFWwindow());

}


InteractiveSystem::~InteractiveSystem(){



}



void InteractiveSystem::registerGlfwCallbacks(){

    // Store camera positioner pointer for callbacks，存进去之后在callback的lambda可以用
    glfwSetWindowUserPointer(window_app.getGLFWwindow(), this);

    // Mouse button callback - prioritize ImGui
    glfwSetMouseButtonCallback(window_app.getGLFWwindow(), 
    [](GLFWwindow* window, int button, int action, int mods){
        auto* intera_system = static_cast<InteractiveSystem*>(glfwGetWindowUserPointer(window));

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        // Handle ImGui input first
        const ImGuiMouseButton_ imguiButton = (button == GLFW_MOUSE_BUTTON_LEFT)
                ? ImGuiMouseButton_Left
                : (button == GLFW_MOUSE_BUTTON_RIGHT ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);

        ImGuiIO& io = ImGui::GetIO();
        // io.MousePos = ImVec2((float)xpos, (float)ypos);
        io.MouseDown[imguiButton] = action == GLFW_PRESS;

        // Only handle camera input if ImGui doesn't want the mouse
        if (!io.WantCaptureMouse) {
            intera_system->camera_Arcball_positioner.onMouseButton(button, action, xpos, ypos);
        }
    });

    glfwSetCursorPosCallback(window_app.getGLFWwindow(), [](GLFWwindow* window, double x, double y) {
        auto* intera_system = static_cast<InteractiveSystem*>(glfwGetWindowUserPointer(window));
        
        // Update ImGui mouse position
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2((float)x, (float)y);
        
        // Only handle camera input if ImGui doesn't want the mouse
        if (!io.WantCaptureMouse) {
            intera_system->camera_Arcball_positioner.onCursorPos(x, y);
        }
    });


}








