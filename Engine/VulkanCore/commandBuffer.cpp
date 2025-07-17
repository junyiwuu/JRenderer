#include "commandBuffer.hpp"




JCommandBuffer::JCommandBuffer(
    JDevice& device,  VkCommandBufferLevel level):
    device_app(device)
        
        //    commandPool(commandPool), device(device), renderPass(renderPass), graphicPipeline(graphicPipeline)
    
{
    createCommandBuffer(device_app, level);

}




JCommandBuffer::~JCommandBuffer(){

        vkFreeCommandBuffers(device_app.device(), device_app.getCommandPool(), 1, &commandBuffer_);


}









void JCommandBuffer::createCommandBuffer(JDevice& device, VkCommandBufferLevel level) 
{
    // commandBuffers_.resize(JSwapchain::MAX_FRAMES_IN_FLIGHT); // x 个commandbuffer在里面

    VkCommandBufferAllocateInfo allocInfo{}; // 告诉x个command buffer进入command pool
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = device_app.getCommandPool();
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;

    //正式把command buffer放入command pool
    VK_CHECK_RESULT (vkAllocateCommandBuffers(device_app.device(), &allocInfo, &commandBuffer_));
        
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










































