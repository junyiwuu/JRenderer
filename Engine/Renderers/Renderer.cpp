#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Renderer.hpp"
#include "../VulkanCore/structs/pushConstants.hpp"
#include "../VulkanCore/structs/uniforms.hpp"


#include "../VulkanCore/window.hpp"
#include "../VulkanCore/device.hpp"




Renderer::Renderer(JWindow& window, JDevice& device):
    window_app(window) ,device_app(device)

{
    init();

}


Renderer::~Renderer(){


}






void Renderer::init() {

    //create swapchain
    swapchain_app = std::make_unique<JSwapchain>(device_app, window_app);

    //create sync objects
    sync_objs.reserve(Global::MAX_FRAMES_IN_FLIGHT);
    for(size_t i=0 ; i<Global::MAX_FRAMES_IN_FLIGHT ; ++i){
        sync_objs.push_back(std::make_unique<JSync>(device_app));
    }

    //create command buffers
    commandBuffers_app.reserve(Global::MAX_FRAMES_IN_FLIGHT);
    for(size_t i =0; i< Global::MAX_FRAMES_IN_FLIGHT; ++i){
        std::unique_ptr<JCommandBuffer> commandBuffer_app = std::make_unique<JCommandBuffer>(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        commandBuffers_app.push_back(std::move(commandBuffer_app));
    }

    imgui_obj = std::make_unique<JImGui>(device_app, *swapchain_app, window_app.getGLFWwindow());
}



//-----------------------------------------------------------------------------------



VkCommandBuffer Renderer::beginFrame(){

    assert(!isFrameStarted && "cant call beginFrame when the frame is still rendering!");
        //阻塞CPU，知道in flight fence关联的那次vk queue submit都执行完毕后，把fence设置为signaled
    //让CPU卡住等GPU完成，因为需要等上一帧的命令执行完才能再重用buffer, image等资源
    vkWaitForFences(device_app.device(), 1, &sync_objs[currentFrame]->inFlightFence, VK_TRUE, UINT64_MAX);

   
    // signal the semaphore，初始化的时候是unsignaled的状态，执行这条后，semaphore从unsignal变成了signal
    VkResult result = vkAcquireNextImageKHR(device_app.device(), swapchain_app->swapChain(), 
                UINT64_MAX, sync_objs[currentFrame]->imageAvailableSemaphore, 
                VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
           // Frame was skipped
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }
    //把fence从signal变成unsignal  // the order is important
    vkResetFences(device_app.device(), 1,  &sync_objs[currentFrame]->inFlightFence);

    isFrameStarted = true;
    commandBuffers_app[currentFrame]->reset();

    // start the command
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(commandBuffers_app[currentFrame]->getCommandBuffer(), &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    return commandBuffers_app[currentFrame]->getCommandBuffer();
}








void Renderer::beginRender(VkCommandBuffer commandBuffer){

    //attachment info 
    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType               = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView           = swapchain_app->getColorImageView();
    colorAttachment.imageLayout         = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveImageView    = swapchain_app->getSwapChainImageView()[imageIndex];
    colorAttachment.resolveImageLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveMode         =  VK_RESOLVE_MODE_AVERAGE_BIT;
    colorAttachment.loadOp              = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp             = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue          = { {0.1f,0.1f,0.1f,1.0f} };

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView   = swapchain_app->getDepthImageView();
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.clearValue  = { {1.0f,0} };

    VkRenderingInfo renderingInfo{};

    renderingInfo.sType                   = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea             = VkRect2D{ {0,0}, {swapchain_app->getSwapChainExtent()} };
    renderingInfo.layerCount             = 1;
    renderingInfo.viewMask               = 0;
    renderingInfo.colorAttachmentCount   = 1;
    renderingInfo.pColorAttachments      = &colorAttachment;
    renderingInfo.pDepthAttachment       = &depthAttachment;      // 或 nullptr
    renderingInfo.pStencilAttachment     = nullptr;

    // layout transfer
    device_app.transitionImageLayout(commandBuffer, swapchain_app->getSwapChainImage()[imageIndex], 
            VK_IMAGE_LAYOUT_UNDEFINED , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, 1);

    device_app.transitionImageLayout(commandBuffer, swapchain_app->getColorImage(), 
            VK_IMAGE_LAYOUT_UNDEFINED , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, 1);
    
    device_app.transitionImageLayout(commandBuffer, swapchain_app->getDepthImage(), 
            VK_IMAGE_LAYOUT_UNDEFINED , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT, 1);



    vkCmdBeginRendering(commandBuffer, &renderingInfo);

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

}
       
        




void Renderer::endRender(VkCommandBuffer commandBuffer){

    vkCmdEndRendering(commandBuffer);

    device_app.transitionImageLayout(commandBuffer, swapchain_app->getSwapChainImage()[imageIndex], 
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
        VK_IMAGE_ASPECT_COLOR_BIT, 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}




void Renderer::endFrame() {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //wait for this semaphore signaled, then execute 
    VkSemaphore waitSemaphores[] = {sync_objs[currentFrame]->imageAvailableSemaphore}; //通过之前的acquire next，这里期待是得到的signaled semaphore
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(commandBuffers_app[currentFrame]->getCommandBuffer());
    VkSemaphore signalSemaphores[] = {sync_objs[currentFrame]->renderFinishedSemaphore};//当前为unsignaled
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // 做queue submit的时候fence必须是Unsignal的状态
    if (vkQueueSubmit(device_app.graphicsQueue(), 1, &submitInfo,  sync_objs[currentFrame]->inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }//GPU执行命令，完成后fence自动变成signaled.
    //也自动把render finished sempahore变成signaled
    //GPU消费semaphore，所以这里的wait semaphore自动从signal 消费成unsignal

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapchain_app->swapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    auto result = vkQueuePresentKHR(device_app.presentQueue()  , &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    isFrameStarted = false;
    currentFrame = (currentFrame + 1) % Global::MAX_FRAMES_IN_FLIGHT;
    // return true;  // Frame was successfully rendered
}




void Renderer::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window_app.getGLFWwindow(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window_app.getGLFWwindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device_app.device());

    if(swapchain_app == nullptr){  //第一次初始化的时候
        swapchain_app =std::make_unique<JSwapchain>(device_app, window_app);
    }else { // 如果不是第一次创建，也就是resize的时候
        std::shared_ptr<JSwapchain> oldSwapChain = std::move(swapchain_app);
        swapchain_app = std::make_unique<JSwapchain>(device_app, window_app, oldSwapChain);

    }

}

