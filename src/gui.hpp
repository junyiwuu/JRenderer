#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "device.hpp"
#include "global.hpp"
#include "buffer.hpp"
#include "commandBuffer.hpp"


class JGui{

public:

    JGui(JDevice& device);
    ~JGui();







private:
    JDevice& device_app;
    VkImage fontImage_ = VK_NULL_HANDLE;
    VkImageView fontImageView_ = VK_NULL_HANDLE;
    VkDeviceMemory fontMemory_ = VK_NULL_HANDLE;

    
    void initResources(VkRenderPass renderPass, VkQueue queue, const std::string& shadersPath);





};