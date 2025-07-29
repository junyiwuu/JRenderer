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
    
    
    
    
private:
    JWindow& window_app;
    JDevice& device_app;
    const JSwapchain& swapchain_app;
    
    Scene::JCameraPositioner_Arcball camera_Arcball_positioner;
    Scene::JCamera camera_Arcball;
    
    //imgui
    std::unique_ptr<JImGui> imgui_obj;




};


































