#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "util.hpp"
#include "shaderModule.hpp"


struct PipelineConfigInfo
{
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
    JPipeline(VkDevice& device ,const std::string& vertFilepath, const std::string& fragFilepath, 
                const PipelineConfigInfo& configInfo);
    ~JPipeline();

    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

    VkPipeline getGraphicPipeline() {return graphicsPipeline_;}







private:
    VkDevice& device;

    VkPipelineLayout pipelineLayout_;
    VkPipeline graphicsPipeline_;

    void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo& configInfo);



















};