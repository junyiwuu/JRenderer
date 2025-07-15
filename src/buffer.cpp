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

void JBuffer::stagingAction(const void* transferData){
    vkMapMemory(device_app.device(), bufferMemory_, 0, size_, 0, &mapped_);  //staging buffer is host access on gpu
    memcpy(mapped_, transferData, (size_t)(size_)); //transfer data is host access, copy to staging buffer
    vkUnmapMemory(device_app.device(), bufferMemory_);  // unmap staging buffer and will be destroyed
}


JBuffer::externalCreateBufferResult JBuffer::createBuffer(JDevice& device_app,  VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties){

    externalCreateBufferResult result;

    VkBuffer buffer;
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device_app.device(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS){
        throw std::runtime_error("failed to create buffer!"); }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device_app.device(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = util::findMemoryType(memRequirements.memoryTypeBits, properties, device_app.physicalDevice() );

    VkDeviceMemory bufferMemory;
    if(vkAllocateMemory(device_app.device(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS){
        throw std::runtime_error("failed to allocate buffer memory!"); }

    vkBindBufferMemory(device_app.device(), buffer,  bufferMemory, 0);

    result.r_buffer_ = buffer;
    result.r_bufferMemory_ = bufferMemory;
    
    return result;
}



void JBuffer::destroyBuffer(JDevice& device_app, VkBuffer buffer, VkDeviceMemory bufferMemory){
    if(buffer != VK_NULL_HANDLE){
        vkDestroyBuffer(device_app.device(), buffer, nullptr); }

    if(bufferMemory != VK_NULL_HANDLE){
        vkFreeMemory(device_app.device(), bufferMemory, nullptr); }

}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////




JUniformBuffer::JUniformBuffer(JDevice& device):
    device_app(device)

{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    // uniformBuffers_.resize(frames);
    // uniformBuffersMemory_.resize(frames);
    // uniformBuffersMapped_.resize(frames);



    auto result = JBuffer::createBuffer(device, bufferSize, 
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uniformBuffer_ = result.r_buffer_ ;
    uniformBufferMemory_ = result.r_bufferMemory_;

    vkMapMemory(device.device(), uniformBufferMemory_, 0, bufferSize, 0, &uniformBufferMapped_);
}



JUniformBuffer::~JUniformBuffer(){

    vkDestroyBuffer(device_app.device(), uniformBuffer_, nullptr);
    vkFreeMemory(device_app.device(), uniformBufferMemory_, nullptr);
    
}





// void JUniformBuffer::update(uint32_t currentImage, const UniformBufferObject& targetUbo){

//     memcpy(uniformBufferMapped_[currentImage], &targetUbo, sizeof(targetUbo));
// }





































