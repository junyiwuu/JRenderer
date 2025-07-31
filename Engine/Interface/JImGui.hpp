#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <memory>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "../VulkanCore/global.hpp"
#include "../VulkanCore/material/load_texture.hpp"
#include "uiSettings.hpp"

class JDevice;
class JSwapchain;
class JDescriptorPool;




// extern UIsettings uiSettings;


class JImGui {
public:
    JImGui(JDevice& device, const JSwapchain& swapchain, GLFWwindow* window, 
            UI::UISettings& uiSettings);
    ~JImGui();

    JImGui(const JImGui&) = delete;
    JImGui& operator=(const JImGui&) = delete;

    void newFrame();
    void render(VkCommandBuffer commandBuffer);
    void endFrame();

    bool wantCaptureKeyboard() const { return ImGui::GetIO().WantCaptureKeyboard; }
    bool wantCaptureMouse() const { return ImGui::GetIO().WantCaptureMouse; }

private:
    JDevice& device_app;
    const JSwapchain& swapchain_app;
    GLFWwindow* window_ptr;
    UI::UISettings& uiSettings;
    
    std::unique_ptr<JDescriptorPool> descriptorPool_obj;

    void setupStyle();
    void createDescriptorPool();

    static void checkVkResult(VkResult err);




    JTexture texture_viewTest;
    ImTextureID texID;
};