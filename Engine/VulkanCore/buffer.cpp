#include "buffer.hpp"
#include "./structs/uniforms.hpp"







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


        //if it is buffer reference --> need device address
        if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT){
            VkMemoryAllocateFlagsInfo allocFlagsInfo{};
            allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
            allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
            allocInfo.pNext = &allocFlagsInfo;
        }



        if(vkAllocateMemory(device_app.device(), &allocInfo, nullptr, &bufferMemory_) != VK_SUCCESS){
            throw std::runtime_error("failed to allocate buffer memory!"); }

        vkBindBufferMemory(device_app.device(), buffer_,  bufferMemory_, 0);

}


uint64_t JBuffer::getBufferAddress() {
    VkBufferDeviceAddressInfo addrInfo{};
    addrInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addrInfo.buffer = buffer_;;
    return vkGetBufferDeviceAddress(device_app.device(), &addrInfo);
}




JBuffer::~JBuffer(){
    if(buffer_ != VK_NULL_HANDLE){
        vkDestroyBuffer(device_app.device(), buffer_, nullptr); }

    if(bufferMemory_ != VK_NULL_HANDLE){
        vkFreeMemory(device_app.device(), bufferMemory_, nullptr); }

}

void JBuffer::map(){
    vkMapMemory(device_app.device(), bufferMemory_, 0, size_, 0, &mapped_);  //在cpu上创建mapped_指针
}

void JBuffer::unmap(){
    if(mapped_!=nullptr){
        vkUnmapMemory(device_app.device(), bufferMemory_);
        mapped_=nullptr;
    }
}


void JBuffer::stagingAction(const void* transferData){
    vkMapMemory(device_app.device(), bufferMemory_, 0, size_, 0, &mapped_);  //staging buffer is host access on gpu
    memcpy(mapped_, transferData, (size_t)(size_)); //transfer data is host access, copy to staging buffer
    vkUnmapMemory(device_app.device(), bufferMemory_);  // unmap staging buffer and will be destroyed
}

void JBuffer::destroyBuffer(JDevice& device_app, VkBuffer buffer, VkDeviceMemory bufferMemory){
    if(buffer != VK_NULL_HANDLE){
        vkDestroyBuffer(device_app.device(), buffer, nullptr); }

    if(bufferMemory != VK_NULL_HANDLE){
        vkFreeMemory(device_app.device(), bufferMemory, nullptr); }

}

VkDescriptorBufferInfo JBuffer::descriptorInfo(VkDeviceSize size , VkDeviceSize offset ){
    return VkDescriptorBufferInfo{
        buffer_,
        offset,
        size_,   };
}


// JBuffer::externalCreateBufferResult JBuffer::createBuffer(JDevice& device_app,  VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties){

//     externalCreateBufferResult result;

//     VkBuffer buffer;
//     VkBufferCreateInfo bufferInfo{};
//     bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//     bufferInfo.size = size;
//     bufferInfo.usage = usage;
//     bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

//     if(vkCreateBuffer(device_app.device(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS){
//         throw std::runtime_error("failed to create buffer!"); }

//     VkMemoryRequirements memRequirements;
//     vkGetBufferMemoryRequirements(device_app.device(), buffer, &memRequirements);

//     VkMemoryAllocateInfo allocInfo{};
//     allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//     allocInfo.allocationSize = memRequirements.size;
//     allocInfo.memoryTypeIndex = util::findMemoryType(memRequirements.memoryTypeBits, properties, device_app.physicalDevice() );

//     VkDeviceMemory bufferMemory;
//     if(vkAllocateMemory(device_app.device(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS){
//         throw std::runtime_error("failed to allocate buffer memory!"); }

//     vkBindBufferMemory(device_app.device(), buffer,  bufferMemory, 0);

//     result.r_buffer_ = buffer;
//     result.r_bufferMemory_ = bufferMemory;
    
//     return result;
// }

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////




// JUniformBuffer::JUniformBuffer(JDevice& device):
//     device_app(device)

// {
//     VkDeviceSize bufferSize = sizeof(UniformBufferObject);

//     // uniformBuffers_.resize(frames);
//     // uniformBuffersMemory_.resize(frames);
//     // uniformBuffersMapped_.resize(frames);



//     auto result = JBuffer::createBuffer(device, bufferSize, 
//         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
//         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

//     uniformBuffer_ = result.r_buffer_ ;
//     uniformBufferMemory_ = result.r_bufferMemory_;

//     vkMapMemory(device.device(), uniformBufferMemory_, 0, bufferSize, 0, &uniformBufferMapped_);
// }



// JUniformBuffer::~JUniformBuffer(){

//     vkDestroyBuffer(device_app.device(), uniformBuffer_, nullptr);
//     vkFreeMemory(device_app.device(), uniformBufferMemory_, nullptr);
    
// }








































