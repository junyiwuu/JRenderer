#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

#include "utility.hpp"
class JDevice;
class JSwapchain;



struct PipelineConfigInfo
{
    PipelineConfigInfo() = default;   
    PipelineConfigInfo(const PipelineConfigInfo& ) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo) = delete;

    VkPipelineViewportStateCreateInfo viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    
    // VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;

};


class JPipeline{
public:
    JPipeline(JDevice& device , const JSwapchain& swapchain,
        const std::string& vertFilepath, const std::string& fragFilepath, 
        const VkPipelineLayout pipelineLayout, const PipelineConfigInfo& configInfo);
    ~JPipeline();

    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

    VkPipeline getGraphicPipeline() const {return graphicsPipeline_;}
private:
    JDevice& device_app;
    // VkDescriptorSetLayout& descriptorSetLayout;
    const JSwapchain& swapchain_app;

    VkPipeline graphicsPipeline_;

    void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, 
        const VkPipelineLayout pipelineLayout, const PipelineConfigInfo& configInfo );
};

/////////////////////////////////////////////////////////////////

class JPipelineLayout{
public:
    class Builder{
        public:
        Builder(JDevice& device): device_app(device) {}
        Builder& setDescriptorSetLayout(uint32_t setLayoutCount, const VkDescriptorSetLayout* pSetLayouts);
        Builder& setPushConstRanges(uint32_t pushConstantRangeCount, const VkPushConstantRange* pPushConstantRanges);
        std::unique_ptr<JPipelineLayout> build() const;

        private:
        JDevice& device_app;
        uint32_t m_setLayoutCount = 0;
        const VkDescriptorSetLayout* m_pSetLayouts = nullptr;
        uint32_t m_pushConstantRangeCount = 0;
        const VkPushConstantRange* m_pPushConstantRanges = nullptr;
    };

    JPipelineLayout(JDevice& device, 
        uint32_t setLayoutCount, const VkDescriptorSetLayout* pSetLayouts,
        uint32_t pushConstantRangeCount, const VkPushConstantRange* pPushConstantRanges);
    ~JPipelineLayout();
    VkPipelineLayout getPipelineLayout() const {return pipelineLayout_;}

private:
    JDevice& device_app;
    VkPipelineLayout pipelineLayout_;

};