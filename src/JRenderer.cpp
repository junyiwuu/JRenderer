#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

#include "window.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "utility.hpp"
#include "shaderModule.hpp"
#include "pipeline.hpp"
#include "commandBuffers.hpp"
#include "buffer.hpp"


const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};


class JRenderer {
public:


    void run() {
        window_app =  std::make_unique<JWindow>(WIDTH, HEIGHT, "vulkan");
        window  = window_app->getGLFWwindow();

        device_app = std::make_unique<JDevice>(*window_app);
        device = device_app->device();
        surface = device_app->surface();
        graphicsQueue = device_app->graphicsQueue();
        presentQueue = device_app->presentQueue();
        physicalDevice = device_app->physicalDevice();
        commandPool = device_app->getCommandPool();

        swapchain_app = std::make_unique<JSwapchain>(*device_app, *window_app);
        swapChain = swapchain_app->swapChain();
 
        PipelineConfigInfo pipelineConfig{};
        JPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = swapchain_app->renderPass();
        pipeline_app = std::make_unique<JPipeline>(device, 
                        "../shaders/shader.vert.spv",
                        "../shaders/shader.frag.spv",
                        pipelineConfig );
        graphicPipeline = pipeline_app->getGraphicPipeline();
    
        vertexBuffer_obj = std::make_unique<JVertexBuffer>(*device_app, vertices, commandPool, graphicsQueue);
        indexBuffer_obj = std::make_unique<JIndexBuffer>(*device_app, indices, commandPool, graphicsQueue);

  
        
        commandBuffers_app = std::make_unique<JCommandBuffers>(*device_app);
        commandBuffers = commandBuffers_app->getCommandBuffers();
        
        
       
        



        mainLoop();
      
    }

private:
    //window
    std::unique_ptr<JWindow> window_app;
    GLFWwindow* window  = nullptr;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    //device
    std::unique_ptr<JDevice> device_app;

    VkDevice device;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkCommandPool commandPool;

    //swapchain
    //one window only need one swapchain
    std::unique_ptr<JSwapchain> swapchain_app;
    VkSwapchainKHR swapChain;

    //pipeline
    std::unique_ptr<JPipeline> pipeline_app;
    VkPipeline graphicPipeline;

    //commandbuffers
    std::unique_ptr<JCommandBuffers> commandBuffers_app;
    std::vector<VkCommandBuffer> commandBuffers;

    uint32_t currentFrame = 0;

    //vertex
    std::unique_ptr<JVertexBuffer> vertexBuffer_obj;
    std::unique_ptr<JIndexBuffer> indexBuffer_obj;
//-----------------------------------------------------------------------------------


    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }
    
        vkDeviceWaitIdle(device);
    
        if(swapchain_app == nullptr){  //第一次初始化的时候
            swapchain_app =std::make_unique<JSwapchain>(*device_app, *window_app);
        }else { // 如果不是第一次创建，也就是resize的时候
            std::shared_ptr<JSwapchain> oldSwapChain = std::move(swapchain_app);
            swapchain_app = std::make_unique<JSwapchain>(*device_app, *window_app, oldSwapChain);

        }

    }
    



    ;

    bool framebufferResized = false;



    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }



   

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapchain_app->renderPass();
        // renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapchain_app->getFrameBuffer(imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchain_app->getSwapChainExtent();

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
 
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) swapchain_app->getSwapChainExtent().width;
            viewport.height = (float) swapchain_app->getSwapChainExtent().height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapchain_app->getSwapChainExtent();
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipeline);
            VkBuffer vertexBuffers[] = {vertexBuffer_obj->baseBuffer.buffer()};
            VkDeviceSize offsets[] = {0};
            //binding
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer,indexBuffer_obj->baseBuffer.buffer(), 0, VK_INDEX_TYPE_UINT16);

            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
            // vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    

    void drawFrame() {
        vkWaitForFences(device, 1, &swapchain_app->getCurrentInFlightFence(currentFrame), VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapchain_app->swapChain(), UINT64_MAX, swapchain_app->getCurrentImageAvailableSemaphore(currentFrame), VK_NULL_HANDLE, &imageIndex);
   

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1,  &swapchain_app->getCurrentInFlightFence(currentFrame));

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {swapchain_app->getCurrentImageAvailableSemaphore(currentFrame)}; //初始化是unsignaled
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {swapchain_app->getCurrentRenderFinishedSemaphore(currentFrame)};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo,  swapchain_app->getCurrentInFlightFence(currentFrame)) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapchain_app->swapChain()};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % JSwapchain::MAX_FRAMES_IN_FLIGHT;
    }

    


};

int main() {
    JRenderer app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
