
#include "shaderModule.hpp"





JShaderModule::JShaderModule(VkDevice device, const std::vector<char>& code):
    device(device), code(code)
{
    createShaderModule(code);

}


JShaderModule::~JShaderModule(){
    vkDestroyShaderModule(device, shaderModule_, nullptr);

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




















