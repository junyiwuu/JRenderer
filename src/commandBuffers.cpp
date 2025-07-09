#include "commandBuffers.hpp"




JCommandBuffers::JCommandBuffers(
    JDevice& device):
    device_app(device)
        
        //    commandPool(commandPool), device(device), renderPass(renderPass), graphicPipeline(graphicPipeline)
    
{
    createCommandBuffers(device_app);

}




JCommandBuffers::~JCommandBuffers(){




}









void JCommandBuffers::createCommandBuffers(JDevice& device) 
{
    commandBuffers_.resize(JSwapchain::MAX_FRAMES_IN_FLIGHT); // x 个commandbuffer在里面

    VkCommandBufferAllocateInfo allocInfo{}; // 告诉x个command buffer进入command pool
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = device_app.getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers_.size();

    //正式把command buffer放入command pool
    if (vkAllocateCommandBuffers(device_app.device(), &allocInfo, commandBuffers_.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}



















































