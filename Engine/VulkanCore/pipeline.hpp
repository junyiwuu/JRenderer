#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>
#include <span>

#include "utility.hpp"
class JDevice;
class JSwapchain;



struct PipelineConfigInfo
{
    PipelineConfigInfo();   
    PipelineConfigInfo(const PipelineConfigInfo& ) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo) = delete;


    VkPipelineViewportStateCreateInfo       viewportInfo;
    VkPipelineInputAssemblyStateCreateInfo  inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo  rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo    multisampleInfo;
    VkPipelineColorBlendStateCreateInfo     colorBlendInfo;
    VkPipelineColorBlendAttachmentState     colorBlendAttachment;
    VkPipelineDepthStencilStateCreateInfo   depthStencilInfo;

    std::vector<VkDynamicState> dynamicStateEnables;
    VkPipelineDynamicStateCreateInfo dynamicStateInfo;
    
    // VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;

    //shader stage
    VkPipelineShaderStageCreateInfo*        pStages;        //need a pointer here
    uint32_t                                stageCount;

    VkPipelineVertexInputStateCreateInfo            vertexInputStateInfo_;
    void setVertexInputState(
            std::span<const VkVertexInputBindingDescription> bindings,  //span require data need to be laid out contiguously in memory
            std::span<const VkVertexInputAttributeDescription> attributes  );

  private:
    void resetVertexInputState();
    //vertex input
    std::vector<VkVertexInputBindingDescription>    bindingDescription_;  //lifetime issue
    std::vector<VkVertexInputAttributeDescription>  attributeDescription_;
    
};


class JPipeline{
public:
//do builder
    
    JPipeline(JDevice& device , const JSwapchain& swapchain,
        const VkPipelineLayout pipelineLayout, const PipelineConfigInfo& configInfo);
    ~JPipeline();

    static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

    VkPipeline getGraphicPipeline() const {return graphicsPipeline_;}
private:
    JDevice& device_app;
    // VkDescriptorSetLayout& descriptorSetLayout;
    const JSwapchain& swapchain_app;

    //some member
    VkPipeline graphicsPipeline_;

    void createGraphicsPipeline( const VkPipelineLayout pipelineLayout, const PipelineConfigInfo& configInfo );
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