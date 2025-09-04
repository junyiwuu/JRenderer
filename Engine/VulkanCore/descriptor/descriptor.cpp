#include "descriptor.hpp"
////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Descriptor Pool /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


JDescriptorPool::JDescriptorPool(JDevice& device_app, uint32_t maxDescriptorSets, VkDescriptorPoolCreateFlags poolCreateFlags,
    const std::vector<VkDescriptorPoolSize> &poolSize):
    device_app(device_app)
{
    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
    descriptorPoolInfo.pPoolSizes = poolSize.data();
    descriptorPoolInfo.maxSets = maxDescriptorSets;
    descriptorPoolInfo.flags = poolCreateFlags;

    if (vkCreateDescriptorPool(device_app.device(), &descriptorPoolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");}
    }


JDescriptorPool::~JDescriptorPool()
{
    vkDestroyDescriptorPool(device_app.device(), descriptorPool_, nullptr);
}


JDescriptorPool::Builder& JDescriptorPool::Builder::reservePoolDescriptors(VkDescriptorType descriptorType, uint32_t descriptorCount){
    b_poolSizes.push_back({descriptorType, descriptorCount});
    return *this;}

JDescriptorPool::Builder& JDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags){
    b_poolCreateFlags = flags; //pool flags是打开或关闭descriptor pool的特性的
    return* this;}


JDescriptorPool::Builder& JDescriptorPool::Builder::setMaxSets(uint32_t descriptorCount){
    b_maxDescriptorSets = descriptorCount;
    return* this;}


std::unique_ptr<JDescriptorPool> JDescriptorPool::Builder::build() const{
    return std::make_unique<JDescriptorPool>(device_app, b_maxDescriptorSets, b_poolCreateFlags, b_poolSizes );
}


bool JDescriptorPool::allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet){
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPool_;
    descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
    descriptorSetAllocInfo.descriptorSetCount = 1;

    if (vkAllocateDescriptorSets(device_app.device(), &descriptorSetAllocInfo, &descriptorSet) != VK_SUCCESS){
        return false;}
    return true;
    // VkResult r = vkAllocateDescriptorSets(device_app.device(), &descriptorSetAllocInfo, &descriptorSet);
    // if (r != VK_SUCCESS) {
    //     std::cerr << "vkAllocateDescriptorSets failed: "
    //               << tools::errorString(r)
    //               << " (" << r << ")\n";
    //     return false;
    // }
    // return true;
}



////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Descriptor Set Layout //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

JDescriptorSetLayout::JDescriptorSetLayout(JDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) 
    : device_app(device), bindings_(bindings)
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
    for (auto kv: bindings_){
        setLayoutBindings.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    layoutInfo.pBindings = setLayoutBindings.data();
 
    if (vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");  }
}



JDescriptorSetLayout::~JDescriptorSetLayout(){
    vkDestroyDescriptorSetLayout(device_app.device(), descriptorSetLayout_, nullptr);
}


JDescriptorSetLayout::Builder& JDescriptorSetLayout::Builder::addBinding(  // by default count=1
    uint32_t binding,  VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t descriptorCount)
{
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorCount = descriptorCount;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.stageFlags = stageFlags;
    bindings[binding] = layoutBinding;
    return *this; // 解指针，否则返回的就是指针。解指针后得到的是当前对象的引用
}


std::unique_ptr<JDescriptorSetLayout>  JDescriptorSetLayout::Builder::build() const {
    return std::make_unique<JDescriptorSetLayout>(device_app, bindings);
}






////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Descriptor Sets /////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
// JDescriptorSets(JDescriptorSetLayout& descriptorSetLayout, JDescriptorPool& descriptorPool);

JDescriptorWriter::JDescriptorWriter(JDescriptorSetLayout& descriptorSetLayout, VkDescriptorPool descriptorPool):
    descriptorSetLayout_{descriptorSetLayout}, descriptorPool_{descriptorPool}
{
    printf("DEBUG: JDescriptorWriter constructor received pool: %p\n", (void*)descriptorPool);
    printf("DEBUG: JDescriptorWriter stored pool: %p\n", (void*)descriptorPool_);
}

// JDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
// JDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);


JDescriptorWriter &JDescriptorWriter::writeBuffer(uint32_t n_binding, const VkDescriptorBufferInfo* bufferInfo){
    assert(descriptorSetLayout_.bindings_.count(n_binding) == 1 && "Descriptor layout does not contain specified binding");
    auto& bindingDescription = descriptorSetLayout_.bindings_[n_binding];

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstBinding = n_binding;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = bindingDescription.descriptorType;
    descriptorWrite.pBufferInfo = bufferInfo;
    
    descriptorWrites_.push_back(descriptorWrite);
    return *this;
}

JDescriptorWriter &JDescriptorWriter::writeImage(uint32_t n_binding, const VkDescriptorImageInfo* imageInfo){
    assert(descriptorSetLayout_.bindings_.count(n_binding) == 1 && "Descriptor layout does not contain specified binding");
    auto& bindingDescription = descriptorSetLayout_.bindings_[n_binding];

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstBinding = n_binding;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = bindingDescription.descriptorType;    
    descriptorWrite.pImageInfo = imageInfo;

    descriptorWrites_.push_back(descriptorWrite);
    return *this;
}


bool JDescriptorWriter::build(VkDescriptorSet& set){

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};

    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPool_;
    descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout_.descriptorSetLayout();
    descriptorSetAllocInfo.descriptorSetCount = 1;

    printf("DEBUG: JDescriptorWriter::build using pool: %p\n", (void*)descriptorPool_);
    if (vkAllocateDescriptorSets(descriptorSetLayout_.device_app.device(), &descriptorSetAllocInfo, &set) != VK_SUCCESS){
        return false;}
    overwrite(set); 
    return true;
}


bool JDescriptorWriter::overwrite(VkDescriptorSet& set){
    for (auto& write: descriptorWrites_){
        write.dstSet = set;  }
    vkUpdateDescriptorSets(descriptorSetLayout_.device_app.device(), 
            descriptorWrites_.size(), descriptorWrites_.data(), 0, nullptr);
    return true;
}






















