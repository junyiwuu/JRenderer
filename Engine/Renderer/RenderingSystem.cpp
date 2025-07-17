

#include "RenderingSystem.hpp"
#include "../VulkanCore/device.hpp"
#include "../VulkanCore/commandBuffer.hpp"




namespace Renderer{



RenderingSystem::RenderingSystem(const JDevice& device) 
    :device_app
{


}


RenderingSystem::~RenderingSystem(){


}




void RenderingSystem::init(){

    commandBuffers_app.reserve(Global::MAX_FRAMES_IN_FLIGHT);
    for(size_t i =0; i< Global::MAX_FRAMES_IN_FLIGHT; ++i){
        std::unique_ptr<JCommandBuffer> commandBuffer_app = std::make_unique<JCommandBuffer>(device_app, vk::CommandBufferLevel::ePrimary);
        commandBuffers_app.push_back(std::move(commandBuffer_app));
    }
}





void RenderingSystem::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    commandBuffers_app[imageIndex]->beginRecording();

    // need transition

    vk::RenderingInfo renderingInfo;
    renderingInfo.renderArea.offset    = vk::Offset2D(0, 0);
    renderingInfo.renderArea.extent    = vk::Extent2D(m_width, m_height);
    renderingInfo.layerCount           = 1;
    renderingInfo.colorAttachmentCount = colourAttachments.size();
    renderingInfo.pColorAttachments    = colourAttachments.data();
    renderingInfo.pDepthAttachment   = &depthAttachment;
    renderingInfo.pStencilAttachment = &depthAttachment;



    
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

    
    // gui->drawFrame(commandBuffer);


    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}





    

void drawFrame() {
    vkWaitForFences(device, 1, &swapchain_app->getCurrentInFlightFence(currentFrame), VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapchain_app->swapChain(), UINT64_MAX, 
            swapchain_app->getCurrentImageAvailableSemaphore(currentFrame), VK_NULL_HANDLE, &imageIndex);


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




















};