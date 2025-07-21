

#include "shaderModule.hpp"
#include "load_model.hpp"
#include "device.hpp"
#include "descriptor.hpp"
#include "pipeline.hpp"
#include "swapchain.hpp"




JPipeline::JPipeline(
    JDevice& device , const JSwapchain& swapchain,
    const std::string& vertFilepath, const std::string& fragFilepath, 
    const VkPipelineLayout pipelineLayout, const PipelineConfigInfo& configInfo ) : 
    
        device_app(device), swapchain_app(swapchain)
{
    createGraphicsPipeline(vertFilepath, fragFilepath,  pipelineLayout ,configInfo);

}


JPipeline::~JPipeline(){
    

    vkDestroyPipeline(device_app.device(), graphicsPipeline_, nullptr);

}




void JPipeline:: createGraphicsPipeline(
        const std::string& vertFilepath, const std::string& fragFilepath, 
        const VkPipelineLayout pipelineLayout, const PipelineConfigInfo& configInfo ) 
        
{
    auto vertShaderCode = util::readFile(vertFilepath);
    auto fragShaderCode = util::readFile(fragFilepath);

    // auto vertShaderModule_obj = std::make_unique<JShaderModule>(device_app.device(), vertShaderCode);
    // VkShaderModule vertShaderModule = vertShaderModule_obj->getShaderModule();
    // auto fragShaderModule_obj = std::make_unique<JShaderModule>(device_app.device(), fragShaderCode);
    // VkShaderModule fragShaderModule = fragShaderModule_obj->getShaderModule();

    JShaderModule vertShaderModule_obj{device_app.device(), vertShaderCode};
    VkShaderModule vertShaderModule = vertShaderModule_obj.getShaderModule();
    JShaderModule fragShaderModule_obj{device_app.device(), fragShaderCode};
    VkShaderModule fragShaderModule = fragShaderModule_obj.getShaderModule();

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescription = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount =  1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size()); 
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();


    VkPipelineRenderingCreateInfo renderingCreateInfo{};
    VkFormat colorFormat = swapchain_app.getSwapChainImageFormat();
    renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderingCreateInfo.colorAttachmentCount = 1;
    renderingCreateInfo.pColorAttachmentFormats = &colorFormat; 
    renderingCreateInfo.depthAttachmentFormat = swapchain_app.findDepthFormat();
    renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
    
    
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = &renderingCreateInfo;

    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
    pipelineInfo.pViewportState = &configInfo.viewportInfo;
    pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
    pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
    pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
    pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;
    pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.subpass = configInfo.subpass;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(device_app.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }


}


void JPipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo){
   
    configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
    
    configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    configInfo.viewportInfo.viewportCount = 1;
    configInfo.viewportInfo.scissorCount = 1;

    configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
    configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    configInfo.rasterizationInfo.lineWidth = 1.0f;
    configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;

    configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
    configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
    
    configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
    configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    configInfo.colorBlendInfo.attachmentCount = 1;
    configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
    configInfo.colorBlendInfo.blendConstants[0] = 0.0f;
    configInfo.colorBlendInfo.blendConstants[1] = 0.0f;
    configInfo.colorBlendInfo.blendConstants[2] = 0.0f;
    configInfo.colorBlendInfo.blendConstants[3] = 0.0f;

    configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
    configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
    configInfo.dynamicStateInfo.flags=0;

    configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.minDepthBounds = 0.0f; // Optional
    configInfo.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
    configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.front = {}; // Optional
    configInfo.depthStencilInfo.back = {}; // Optional
    

}



//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

JPipelineLayout::JPipelineLayout(JDevice& device, 
    uint32_t setLayoutCount, const VkDescriptorSetLayout* pSetLayouts,
    uint32_t pushConstantRangeCount, const VkPushConstantRange* pPushConstantRanges):
        device_app(device)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = setLayoutCount;
    pipelineLayoutInfo.pSetLayouts = pSetLayouts;
    pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = pPushConstantRanges;

    if (vkCreatePipelineLayout(device_app.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

}



JPipelineLayout::~JPipelineLayout(){
    vkDestroyPipelineLayout(device_app.device(), pipelineLayout_, nullptr);

}

JPipelineLayout::Builder& JPipelineLayout::Builder::setDescriptorSetLayout(uint32_t setLayoutCount, const VkDescriptorSetLayout* pSetLayouts){
    m_setLayoutCount = setLayoutCount;
    m_pSetLayouts = pSetLayouts;
    return *this;
}

JPipelineLayout::Builder& JPipelineLayout::Builder::setPushConstRanges(uint32_t pushConstantRangeCount, const VkPushConstantRange* pPushConstantRanges){
    m_pushConstantRangeCount = pushConstantRangeCount;
    m_pPushConstantRanges = pPushConstantRanges;
    return *this;
}

std::unique_ptr<JPipelineLayout> JPipelineLayout::Builder::build() const{
    return std::make_unique<JPipelineLayout>(device_app, 
        m_setLayoutCount, m_pSetLayouts, 
        m_pushConstantRangeCount, m_pPushConstantRanges);
}
























