#pragma once

#include <vulkan/vulkan.hpp>
#include "../Scene/camera.hpp"
#include "../Interface/JImGui.hpp"

class JWindow;
class JDevice;
class JSwapchain;



//initiate viewport camera, imgui

class InteractiveSystem{

public:

    InteractiveSystem(JWindow& window, JDevice& device, const JSwapchain& swapchain);
    ~InteractiveSystem();


    void registerGlfwCallbacks();
    JImGui& getImguiApp()                           {return *imgui_obj;}
    Scene::JCamera& getArcballCamera() {return camera_Arcball;}
    
    const glm::mat4 getProjMatrix(float ratio);
    const glm::mat4 getViewMatrix();

    
    
private:
    JWindow& window_app;
    JDevice& device_app;
    const JSwapchain& swapchain_app;
    
    Scene::JCameraPositioner_Arcball camera_Arcball_positioner;
    Scene::JCamera camera_Arcball;

    Scene::JCameraPositioner_firstPerson camera_FPerson_positioner;
    Scene::JCamera camera_FPerson;
    
    //imgui
    std::unique_ptr<JImGui> imgui_obj;


    std::shared_ptr<UI::UISettings> uiSettings;




};


































