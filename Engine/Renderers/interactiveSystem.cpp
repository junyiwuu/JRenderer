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
        glm::vec3(0.0f, 1.0f, -3.0f), 
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f),
        Scene::DragMode::None},
    camera_Arcball{camera_Arcball_positioner},
    camera_FPerson_positioner{
      
        glm::vec3(0.0f, 0.0f, -2.0f), 
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f),
        },
    camera_FPerson{camera_FPerson_positioner}
    

{
    uiSettings = std::make_shared<UI::UISettings>();
    imgui_obj = std::make_unique<JImGui>(device_app, swapchain_app, window_app.getGLFWwindow(), *uiSettings);
    
    assert(&camera_Arcball_positioner == camera_Arcball.debug_getRawPositioner());
    std::cout << "Arcball positioner addr: " << &camera_Arcball_positioner
              << ", camera holds: " << camera_Arcball.debug_getRawPositioner() << "\n";
    
}


InteractiveSystem::~InteractiveSystem(){


}



void InteractiveSystem::registerGlfwCallbacks(){

    // Store camera positioner pointer for callbacks，存进去之后在callback的lambda可以用
    glfwSetWindowUserPointer(window_app.getGLFWwindow(), this);

  if (uiSettings->userCam == UI::UserCam::ArcballCamera){
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
            intera_system->camera_Arcball_positioner.onCursorPos(x, y, window);
        }
    });

  }else if(uiSettings->userCam == UI::UserCam::FirstPersonCamera){
        

        glfwSetMouseButtonCallback(window_app.getGLFWwindow(),
                [](GLFWwindow* window, int button, int action, int mods){

            auto* intera_system = static_cast<InteractiveSystem*>(glfwGetWindowUserPointer(window));

            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            const ImGuiMouseButton_ imguiButton = (button == GLFW_MOUSE_BUTTON_LEFT)
                                            ? ImGuiMouseButton_Left
                                            : (button == GLFW_MOUSE_BUTTON_RIGHT ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);



            ImGuiIO& io                 = ImGui::GetIO();
            io.MousePos                 = ImVec2(  (float)xpos , (float)ypos   );
            io.MouseDown[imguiButton]   = action == GLFW_PRESS;

        if (!io.WantCaptureMouse) {
            intera_system->camera_FPerson_positioner.onMouseButton(button, action, xpos, ypos);
        }

        });


        // glfwSetKeyCallback(window_app.getGLFWwindow(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        //     auto* intera_system = static_cast<InteractiveSystem*>(glfwGetWindowUserPointer(window));
        //     intera_system->camera_FPerson_positioner.onKeyboardButton(key, action);

        //     const bool pressed = action != GLFW_RELEASE;
        //     if (key == GLFW_KEY_ESCAPE && pressed)
        //         glfwSetWindowShouldClose(window, GLFW_TRUE);

        //     if (mods & GLFW_MOD_SHIFT)
        //         cameraPositioner.movement_.fastSpeed_ = pressed;
        //     if (key == GLFW_KEY_F) {
        //         cameraPositioner.lookAt(initialCamPos, initialCamTarget, glm::vec3(0.0f, 1.0f, 0.0f));
        //         cameraPositioner.setSpeed(glm::vec3(0)); }

            
        //     });

        // glfwSetCursorPosCallback(window_app.getGLFWwindow(),    //只有当鼠标移动，才会执行lambda注册的callback，就是mousestae那两行
        //         [](GLFWwindow* window, double x, double y){
        //     int width, height;
        //     glfwGetFramebufferSize(window, &width, &height);
        //     mouseState.pos.x = static_cast<float>(x/width);
        //     mouseState.pos.y = 1.f - static_cast<float>(y/height);

        //     // ImGuiIO& io                 = ImGui::GetIO();
        //     // io.MousePos                 = ImVec2(  (float)x , (float)y   );
        // });
        // //      button detection

            







    }
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































