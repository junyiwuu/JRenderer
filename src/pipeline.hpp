#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "utility.hpp"
#include "shaderModule.hpp"
#include "load_model.hpp"

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
                const PipelineConfigInfo& configInfo, const VkDescriptorSetLayout& descriptorSetLayout);
    ~JPipeline();

    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

    VkPipeline getGraphicPipeline() {return graphicsPipeline_;}
    VkPipelineLayout getPipelineLayout(){ return pipelineLayout_;}

        // graphic pipeline并非固定一个pipelinelayout，layout其实应该和descriptor set相关，之后要整理




private:
    VkDevice& device;
    // VkDescriptorSetLayout& descriptorSetLayout;


    VkPipelineLayout pipelineLayout_;
    VkPipeline graphicsPipeline_;

    void createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, 
            const PipelineConfigInfo& configInfo, const VkDescriptorSetLayout& descriptorSetLayout);



















};