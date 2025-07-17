#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_raii.hpp>
#include <array>
#include <vector>
#include <iostream>
#include <cstring>
#include <optional>
#include <set>
#include <memory>

#include "../global.hpp"

class JSwapchain;
class JCommandBuffer;
class JDevice;





namespace Renderer{

class RenderingSystem{

public:

    RenderingSystem(const JDevice& device);
    ~RenderingSystem();


    void init();
    void Render();
    void Update();

private:
    JDevice& device_app;

    //commandbuffers
    std::vector<std::unique_ptr<JCommandBuffer>> commandBuffers_app;
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);



};




}