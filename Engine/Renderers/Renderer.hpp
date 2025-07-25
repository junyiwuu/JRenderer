#pragma once
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>


#include "../VulkanCore/swapchain.hpp"
#include "../VulkanCore/utility.hpp"
#include "../VulkanCore/shaderModule.hpp"
#include "../VulkanCore/commandBuffer.hpp"
#include "../VulkanCore/buffer.hpp"
#include "../Interface/JImGui.hpp"
#include "../VulkanCore/sync.hpp"
#include "../VulkanCore/global.hpp"

struct UniformBufferObject;
class JWindow;
class JDevice;

class Renderer {
    //Renderer purpose: initiate swapchain, sync, commandbuffer.  define how to draw the frame
public:

    Renderer(JWindow& window, JDevice& device);
    ~Renderer();

    NO_COPY(Renderer);

    //main commmands
    VkCommandBuffer beginFrame();
    void endFrame();
    void beginRender(VkCommandBuffer commandBuffer);
    void endRender(VkCommandBuffer commandBuffer);

    //getter
    const JSwapchain& getSwapchainApp() const       {return *swapchain_app;}
    JImGui& getImguiApp()                           {return *imgui_obj;}
    const uint32_t& getCurrentFrame() const         {return currentFrame;}
    float getSwapchainImageAspectRatio() const      {return swapchain_app->getAspectRatio();}




private:
    JDevice& device_app;
    JWindow& window_app;

    //swapchain
    //one window only need one swapchain
    std::unique_ptr<JSwapchain> swapchain_app;

    //sync objects
    std::vector<std::unique_ptr<JSync>> sync_objs;

    //commandbuffers
    std::vector<std::unique_ptr<JCommandBuffer>> commandBuffers_app;

    uint32_t currentFrame = 0;
    uint32_t imageIndex;  

    bool framebufferResized = false;
    bool isFrameStarted{false};


    void init();
    void recreateSwapChain();

    //imgui
    std::unique_ptr<JImGui> imgui_obj;

};

