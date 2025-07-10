#include "descriptor.hpp"



JDescriptorPool::JDescriptorPool(VkDevice& device, int frames):
    device_(device)
{
    createDescriptorPool(device, frames);

}

JDescriptorPool::~JDescriptorPool()
{
    vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);
}




void JDescriptorPool::createDescriptorPool(VkDevice& device,  int frames) {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(frames);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(frames);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}



////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


JDescriptorSets::JDescriptorSets(VkDevice& device, JDescriptorPool& descriptorPool_obj, 
        int frames, std::vector<VkBuffer> uniformBuffers):
    device_(device)
{
    createDescriptorSetLayout(device);
    createDescriptorSets(device, descriptorPool_obj,frames, uniformBuffers);
}

JDescriptorSets::~JDescriptorSets()
{
    vkDestroyDescriptorSetLayout(device_, descriptorSetLayout_, nullptr);
}




void JDescriptorSets::createDescriptorSetLayout(VkDevice& device) {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}



void JDescriptorSets::createDescriptorSets(VkDevice& device, JDescriptorPool& descriptorPool_obj,int frames,
        std::vector<VkBuffer> uniformBuffers) 
{
    std::vector<VkDescriptorSetLayout> layouts(frames, descriptorSetLayout_);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool_obj.descriptorPool();
    allocInfo.descriptorSetCount = static_cast<uint32_t>(frames);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets_.resize(frames);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets_.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < frames; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets_[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

























