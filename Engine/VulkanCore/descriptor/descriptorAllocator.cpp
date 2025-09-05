#include "descriptorAllocator.hpp"
#include "../device.hpp"




// 初始化的时候先预估一个pool size
JDescriptorAllocator::JDescriptorAllocator(
            JDevice& device, 
            const std::vector<VkDescriptorPoolSize>& p_poolSizes,
            uint32_t initialSetsCount  ):
    device_app(device), p_poolSizes_(p_poolSizes), maxSetsCounter_(initialSetsCount)
{
    // when initialize DescriptorAllocator one pool with at least 10 set is created
    currentPool_ = createPool(initialSetsCount);
    printf("DEBUG: Created descriptor pool: %p\n", (void*)currentPool_);
}


JDescriptorAllocator::~JDescriptorAllocator(){
    vkDestroyDescriptorPool(device_app.device(), currentPool_, nullptr);
    if(usedPool_.size() != 0){
        for (auto pool : usedPool_){
            vkDestroyDescriptorPool(device_app.device(), pool, nullptr);
        }
    }

}




// descriptor pool 动态扩容简单版本：失败时自动扩容


VkDescriptorSet JDescriptorAllocator::allocateDescriptorSet(VkDescriptorSetLayout descriptorSetLayout){
    VkDescriptorSet descriptorSet;

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = currentPool_;
    descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
    descriptorSetAllocInfo.descriptorSetCount = 1;

    VkResult result = vkAllocateDescriptorSets(device_app.device(), &descriptorSetAllocInfo, &descriptorSet);

    if (result == VK_ERROR_FRAGMENTED_POOL || result == VK_ERROR_OUT_OF_POOL_MEMORY){
        //get rid of current pool
        usedPool_.push_back(currentPool_);

        maxSetsCounter_++; 
        VkDescriptorPool newPool_ = createPool(maxSetsCounter_);
        currentPool_ = newPool_;
        descriptorSetAllocInfo.descriptorPool = currentPool_;
        
        if (vkAllocateDescriptorSets(device_app.device(), &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS){
            throw std::runtime_error("failed to allocate the descriptor set even after expanding the descriptor pool");
        };       
    }
    else if (result != VK_SUCCESS){
        throw std::runtime_error("failed to allocate the descriptor set (through descriptor allocator)");
    }
    return descriptorSet;
}




//  set有多少是和有多少material相关的，所以适合set layout相关的？
// 之后应该根据之前的cache来调整，现在先直接根据set layout
VkDescriptorPool JDescriptorAllocator::createPool(uint32_t maxSetsCounter_){

    //poolsize: {descriptor type, count}
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(p_poolSizes_.size());
    descriptorPoolInfo.pPoolSizes = p_poolSizes_.data();
    descriptorPoolInfo.maxSets = maxSetsCounter_;
    descriptorPoolInfo.flags = 0;

    VkDescriptorPool pool;
    if (vkCreateDescriptorPool(device_app.device(), &descriptorPoolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");}
    
    return pool;
}




















