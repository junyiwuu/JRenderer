#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <memory>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "../VulkanCore/global.hpp"

class JDevice;
class JSwapchain;
class JDescriptorPool;


struct UIsettings{
    bool cullBackFace = false;

};
// extern UIsettings uiSettings;


class JImGui {
public:
    JImGui(JDevice& device, JSwapchain& swapchain, GLFWwindow* window);
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
    JSwapchain& swapchain_app;
    GLFWwindow* window_ptr;
    
    std::unique_ptr<JDescriptorPool> descriptorPool_obj;

    void setupStyle();
    void createDescriptorPool();

    static void checkVkResult(VkResult err);

    UIsettings uiSettings;
};