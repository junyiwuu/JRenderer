#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <stdexcept>
#include <optional>
#include "utility.hpp"

class JDevice;

//----------------------
class JShaderModule{
public:
    JShaderModule(VkDevice device, const std::vector<char>& code);
    ~JShaderModule();
    
    // Move constructor and assignment
    JShaderModule(JShaderModule&& other) noexcept;
    JShaderModule& operator=(JShaderModule&& other) noexcept;
    
    // Delete copy constructor and assignment
    JShaderModule(const JShaderModule&) = delete;
    JShaderModule& operator=(const JShaderModule&) = delete;

    VkShaderModule getShaderModule() {return shaderModule_;}

private:
    VkDevice device;
    const std::vector<char>& code;

    VkShaderModule shaderModule_;
    void createShaderModule(const std::vector<char>& code);
};


//-----------------------------------
class JShaderStages{

public:
    class Builder{
      public:
        Builder(JDevice& device):device_app(device ){}
        Builder& setVert(const std::string& filepath);
        Builder& setFrag(const std::string& filepath);
        JShaderStages build();
      private:
        JDevice& device_app;

        //vert
        VkPipelineShaderStageCreateInfo vertShaderStageInfo_{};
        std::optional<JShaderModule> vertShaderModule_{};
        bool vertActive_ = false;

        //frag
        VkPipelineShaderStageCreateInfo fragShaderStageInfo_{};
        std::optional<JShaderModule> fragShaderModule_{};
        bool fragActive_ = false;
    };

public:
    //getter
    std::vector<VkPipelineShaderStageCreateInfo>& getStageInfos() {
        return stageInfos_;
    }

private:
    std::vector<VkPipelineShaderStageCreateInfo> stageInfos_;
    std::vector<JShaderModule> shaderModules_;

    JShaderStages() = default;
    friend class Builder;
};

    




