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
    
    // Delegate methods for AppContext callbacks
    void handleMouseButton(int button, int action, double xpos, double ypos);
    void handleCursorPos(double x, double y, GLFWwindow* window);
    void handleKeyboard(int key, int scancode, int action, int mods);

    
    
private:
    JWindow& window_app;
    JDevice& device_app;
    const JSwapchain& swapchain_app;
    
    std::shared_ptr<Scene::JCameraPositioner_Arcball> camera_Arcball_positioner;
    Scene::JCamera camera_Arcball;

    std::shared_ptr<Scene::JCameraPositioner_firstPerson> camera_FPerson_positioner;
    Scene::JCamera camera_FPerson;
    
    //imgui
    std::unique_ptr<JImGui> imgui_obj;


    std::shared_ptr<UI::UISettings> uiSettings;




};


































