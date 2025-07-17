#include "commandBuffer.hpp"




JCommandBuffer::JCommandBuffer( JDevice& device,  vk::CommandBufferLevel level):
    device_app(device)
    
{
    createCommandBuffer(device_app, level); 
}


void JCommandBuffer::createCommandBuffer(JDevice& device, vk::CommandBufferLevel level) 
{
    // commandBuffers_.resize(JSwapchain::MAX_FRAMES_IN_FLIGHT); // x 个commandbuffer在里面
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool = device_app.getCommandPool();
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    commandBuffer_v = device_app.getDevice_v().allocateCommandBuffers(allocInfo)[0];
    cmdBuf_renderingNow = false;
}




JCommandBuffer::~JCommandBuffer(){

        // vkFreeCommandBuffers(device_app.getDevice(), device_app.getCommandPool(), 1, &commandBuffer_);


}



void JCommandBuffer::beginRecording(){
    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    // beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    cmdBuf_renderingNow = true;
    commandBuffer_v.begin(beginInfo);
}

void JCommandBuffer::endRendering(){
    assert(cmdBuf_renderingNow==true);
    cmdBuf_renderingNow = false;
    commandBuffer_v.end();

}





void JCommandBuffer::beginSingleTimeCommands(){

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer_, &beginInfo));
}




void JCommandBuffer::endSingleTimeCommands(VkQueue queue){
        
    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer_));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer_;

    vkQueueSubmit(queue, 1 , &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    // vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}










































