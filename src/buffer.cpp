#include "buffer.hpp"


JBuffer::JBuffer(JDevice& device,  VkDeviceSize size, 
    VkBufferUsageFlags usage, VkMemoryPropertyFlags properties):
        device_app(device), size_(size)
    
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if(vkCreateBuffer(device_app.device(), &bufferInfo, nullptr, &buffer_) != VK_SUCCESS){
            throw std::runtime_error("failed to create buffer!"); }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device_app.device(), buffer_, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = util::findMemoryType(memRequirements.memoryTypeBits, properties, device_app.physicalDevice() );

        if(vkAllocateMemory(device_app.device(), &allocInfo, nullptr, &bufferMemory_) != VK_SUCCESS){
            throw std::runtime_error("failed to allocate buffer memory!"); }

        vkBindBufferMemory(device_app.device(), buffer_,  bufferMemory_, 0);

}



JBuffer::~JBuffer(){
    if(buffer_ != VK_NULL_HANDLE){
        vkDestroyBuffer(device_app.device(), buffer_, nullptr); }

    if(bufferMemory_ != VK_NULL_HANDLE){
        vkFreeMemory(device_app.device(), bufferMemory_, nullptr); }

}


























































