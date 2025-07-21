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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>



#include "./VulkanCore/window.hpp"
#include "./VulkanCore/device.hpp"
#include "./VulkanCore/swapchain.hpp"
#include "./VulkanCore/utility.hpp"
#include "./VulkanCore/shaderModule.hpp"
#include "./VulkanCore/pipeline.hpp"
#include "./VulkanCore/commandBuffer.hpp"
#include "./VulkanCore/buffer.hpp"
#include "./VulkanCore/descriptor.hpp"
#include "./VulkanCore/load_texture.hpp"
#include "./VulkanCore/load_model.hpp"





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
 
        vikingTexture_obj = std::make_unique<JTexture>("../assets/viking_room.png", *device_app);
        vikingModel_obj = std::make_unique<JModel>("../assets/viking_room.obj");

        vertexBuffer_obj = std::make_unique<JVertexBuffer>(*device_app, vikingModel_obj->vertices(), commandPool, graphicsQueue);
        indexBuffer_obj = std::make_unique<JIndexBuffer>(*device_app, vikingModel_obj->indices(), commandPool, graphicsQueue);

        descriptorSetLayout_obj = JDescriptorSetLayout::Builder{*device_app}
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
            .build();
        descriptorPool_obj  = JDescriptorPool::Builder{*device_app}
            .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3)
            .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6)
            .setMaxSets(6)
            .build();
    
        

        uniformBuffer_objs.reserve(JSwapchain::MAX_FRAMES_IN_FLIGHT);
        descriptorSets.resize(JSwapchain::MAX_FRAMES_IN_FLIGHT);
        for(size_t i =0; i< JSwapchain::MAX_FRAMES_IN_FLIGHT; ++i){
            
            uniformBuffer_objs.emplace_back( std::make_unique<JUniformBuffer>(*device_app) );
            auto& ubo = *uniformBuffer_objs.back();
            JDescriptorWriter writer{*descriptorSetLayout_obj, *descriptorPool_obj };

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = ubo.buffer();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = vikingTexture_obj->textureImageView();
            imageInfo.sampler = vikingTexture_obj->textureSampler();

            if( !writer
                        .writeBuffer(0, &bufferInfo)
                        .writeImage(1, &imageInfo)
                        .build(descriptorSets[i])){
                throw std::runtime_error("failed to allocate descriptor set!");    }  
        }

        PipelineConfigInfo pipelineConfig{};
        JPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = swapchain_app->renderPass();
        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        pipelineConfig.multisampleInfo.rasterizationSamples = device_app->msaaSamples();

        VkDescriptorSetLayout setLayouts[] = {descriptorSetLayout_obj->descriptorSetLayout()};
        pipelinelayout_app = JPipelineLayout::Builder{*device_app}
                            .setDescriptorSetLayout(1, setLayouts)
                            .build();

        pipeline_app = std::make_unique<JPipeline>(*device_app, 
                        "../shaders/shader.vert.spv", "../shaders/shader.frag.spv",
                        pipelinelayout_app->getPipelineLayout(), pipelineConfig);
        graphicPipeline = pipeline_app->getGraphicPipeline();



        commandBuffers_app.reserve(JSwapchain::MAX_FRAMES_IN_FLIGHT);
        for(size_t i =0; i< JSwapchain::MAX_FRAMES_IN_FLIGHT; ++i){
            std::unique_ptr<JCommandBuffer> commandBuffer_app = std::make_unique<JCommandBuffer>(*device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
            commandBuffers_app.push_back(std::move(commandBuffer_app));
        }
        
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
    std::unique_ptr<JPipelineLayout> pipelinelayout_app;
    VkPipeline graphicPipeline;

    //commandbuffers
    std::vector<std::unique_ptr<JCommandBuffer>> commandBuffers_app;
    

    uint32_t currentFrame = 0;


    // const std::vector<Vertex> vertices ;
    // const std::vector<uint32_t> indices ;
    //vertex, index, uniformBuffer
    std::unique_ptr<JVertexBuffer> vertexBuffer_obj;
    std::unique_ptr<JIndexBuffer> indexBuffer_obj;
    std::vector<std::unique_ptr<JUniformBuffer>> uniformBuffer_objs;

    //descriptor
    std::unique_ptr<JDescriptorPool> descriptorPool_obj;
    std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_obj;//现在只用一种layout.如果需要不同的layout就设置多个
    std::vector<VkDescriptorSet> descriptorSets;
    
    //texture
    std::unique_ptr<JTexture> vikingTexture_obj;
    std::unique_ptr<JModel> vikingModel_obj;


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
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

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


        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();



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
            vkCmdBindIndexBuffer(commandBuffer,indexBuffer_obj->baseBuffer.buffer(), 0, VK_INDEX_TYPE_UINT32);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout_app->getPipelineLayout(),0,1,
                        &descriptorSets[currentFrame], 0, nullptr );

            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(vikingModel_obj->indices().size()), 1, 0, 0, 0);
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

        //--------- update uniform buffer
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float updateUniformBuffer_time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), updateUniformBuffer_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapchain_app->getSwapChainExtent().width / (float)swapchain_app->getSwapChainExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        memcpy(uniformBuffer_objs[currentFrame]->bufferMapped(), &ubo, sizeof(ubo) );
        // uniformBuffer_obj->update(currentFrame, ubo);
        recordCommandBuffer(commandBuffers_app[currentFrame]->getCommandBuffer(), imageIndex);
        //---------------------------------------

        vkResetFences(device, 1,  &swapchain_app->getCurrentInFlightFence(currentFrame));

        // vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {swapchain_app->getCurrentImageAvailableSemaphore(currentFrame)}; //初始化是unsignaled
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(commandBuffers_app[currentFrame]->getCommandBuffer());

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
