#include "interactiveSystem.hpp"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <iostream>

#include "../VulkanCore/window.hpp"
#include "../VulkanCore/device.hpp"
#include "../VulkanCore/swapchain.hpp"




InteractiveSystem::InteractiveSystem(JWindow& window, JDevice& device, const JSwapchain& swapchain):
    window_app(window),
    device_app(device),
    swapchain_app(swapchain)  ,
    camera_Arcball_positioner(std::make_shared<Scene::JCameraPositioner_Arcball>(
        glm::vec3(0.0f, 1.0f, -3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        Scene::DragMode::None)),
    camera_Arcball(*camera_Arcball_positioner),
    camera_FPerson_positioner(std::make_shared<Scene::JCameraPositioner_firstPerson>(
        glm::vec3(0.0f, 0.0f, -2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f))),
    camera_FPerson(*camera_FPerson_positioner)
{




    uiSettings = std::make_shared<UI::UISettings>();
    imgui_obj = std::make_unique<JImGui>(device_app, swapchain_app, window_app.getGLFWwindow(), *uiSettings);
    
    // assert(&camera_Arcball_positioner == camera_Arcball.debug_getRawPositioner());
    // std::cout << "Arcball positioner addr: " << &camera_Arcball_positioner
    //           << ", camera holds: " << camera_Arcball.debug_getRawPositioner() << "\n";
    
}


InteractiveSystem::~InteractiveSystem(){


}


const glm::mat4 InteractiveSystem::getProjMatrix(float ratio){

    if(uiSettings->userCam == UI::UserCam::ArcballCamera){
        // std::cerr << "DEBUG: InteractiveSystem calling arcball getProjMatrix" << std::endl;
        return camera_Arcball.getProjMatrix(ratio);
    }else if(uiSettings->userCam == UI::UserCam::FirstPersonCamera){
        // std::cerr << "DEBUG: InteractiveSystem calling first-person getProjMatrix" << std::endl;
        return camera_FPerson.getProjMatrix(ratio); 
    }
    
    // Default fallback to arcball camera
    // std::cerr << "DEBUG: InteractiveSystem calling arcball getProjMatrix (fallback)" << std::endl;
    return camera_Arcball.getProjMatrix(ratio);
}


const glm::mat4 InteractiveSystem::getViewMatrix(){
    if(uiSettings->userCam == UI::UserCam::ArcballCamera){
        return camera_Arcball.getViewMatrix();
    }else if(uiSettings->userCam == UI::UserCam::FirstPersonCamera){
        return camera_FPerson.getViewMatrix(); 
    }

    // Default fallback to arcball camera
    return camera_Arcball.getViewMatrix();
}

// Delegate methods for AppContext callbacks
void InteractiveSystem::handleMouseButton(int button, int action, double xpos, double ypos) {
    if (uiSettings->userCam == UI::UserCam::ArcballCamera) {
        if (camera_Arcball_positioner) {
            camera_Arcball_positioner->onMouseButton(button, action, xpos, ypos);
        }
    } else if (uiSettings->userCam == UI::UserCam::FirstPersonCamera) {
        if (camera_FPerson_positioner) {
            camera_FPerson_positioner->onMouseButton(button, action, xpos, ypos);
        }
    }
}

void InteractiveSystem::handleCursorPos(double x, double y, GLFWwindow* window) {
    if (uiSettings->userCam == UI::UserCam::ArcballCamera) {
        if (camera_Arcball_positioner) {
            camera_Arcball_positioner->onCursorPos(x, y, window);
        }
    } else if (uiSettings->userCam == UI::UserCam::FirstPersonCamera) {
        // First person camera cursor handling would go here

        
    }
}

void InteractiveSystem::handleKeyboard(int key, int scancode, int action, int mods) {
    if (uiSettings->userCam == UI::UserCam::FirstPersonCamera) {
        if (camera_FPerson_positioner) {
            camera_FPerson_positioner->onKeyboardButton(key, action);
        }
    }
    
    // Handle global keys
    const bool pressed = action != GLFW_RELEASE;
    if (key == GLFW_KEY_ESCAPE && pressed) {
        // Could signal app to close, but we don't have direct access to window here
    }
}































