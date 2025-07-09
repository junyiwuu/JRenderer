#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>


class JShaderModule{


public:




    JShaderModule(VkDevice& device, const std::vector<char>& code);
    ~JShaderModule();

    VkShaderModule getShaderModule() {return shaderModule_;}








private:
    VkDevice& device;
    const std::vector<char>& code;

    VkShaderModule shaderModule_;


    void createShaderModule(const std::vector<char>& code);























};