
#include "shaderModule.hpp"
#include "device.hpp"





JShaderModule::JShaderModule(VkDevice device, const std::vector<char>& code):
    device(device), code(code)
{
    createShaderModule(code);

}


JShaderModule::~JShaderModule(){
    if (shaderModule_ != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, shaderModule_, nullptr);
    }
}


// need to revise this part, what is actually we want to move, but it did copy?
JShaderModule::JShaderModule(JShaderModule&& other) noexcept
    : device(other.device), code(other.code), shaderModule_(other.shaderModule_) {
    other.shaderModule_ = VK_NULL_HANDLE;
}

JShaderModule& JShaderModule::operator=(JShaderModule&& other) noexcept {
    if (this != &other) {
        if (shaderModule_ != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device, shaderModule_, nullptr);
        }
        device = other.device;
        // code is a const reference, can't reassign it - this is a design issue
        shaderModule_ = other.shaderModule_;
        other.shaderModule_ = VK_NULL_HANDLE;
    }
    return *this;
}


void JShaderModule::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }
}




 JShaderStages::Builder& JShaderStages::Builder::setVert(const std::string& filepath){
    auto code = util::readFile(filepath);
    vertShaderModule_.emplace(device_app.device(), code);
    
    VkPipelineShaderStageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    info.module = vertShaderModule_->getShaderModule();
    info.pName = "main";

    vertShaderStageInfo_ = info;
    vertActive_ = true;
    return *this;
 }


 JShaderStages::Builder& JShaderStages::Builder::setFrag(const std::string& filepath){
    auto code = util::readFile(filepath);
    fragShaderModule_.emplace(device_app.device(), code);

    VkPipelineShaderStageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    info.module = fragShaderModule_->getShaderModule();
    info.pName = "main";

    fragShaderStageInfo_  = info;
    fragActive_ = true;
    return *this;
 }


 JShaderStages JShaderStages::Builder::build(){
    JShaderStages stages;
    if(vertActive_){
        stages.stageInfos_.push_back(vertShaderStageInfo_);
        stages.shaderModules_.push_back(std::move(*vertShaderModule_));
    }
    if(fragActive_){
        stages.stageInfos_.push_back(fragShaderStageInfo_);
        stages.shaderModules_.push_back(std::move(*fragShaderModule_));
    }
    return stages;
 

 }















