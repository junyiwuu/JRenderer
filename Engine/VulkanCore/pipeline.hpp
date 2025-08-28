#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>
#include <span>
#include <cstdint>

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


// struct ComputePipelineConfigInfo
// {
//   ComputePipelineConfigInfo();
//   ComputePipelineConfigInfo(const ComputePipelineConfigInfo&) = delete;
//   ComputePipelineConfigInfo& operator=(const ComputePipelineConfigInfo) = delete;

//   VkSpecializationInfo specializationInfo;


// };





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


/////////////////////////////////////////////////////////////////

class JComputePipeline{

  public:
    JComputePipeline(JDevice& device, const JShaderModule& shaderModule, const VkPipelineLayout pipelineLayout);
    ~JComputePipeline();

    VkPipeline getComputePipeline() const {return computePipeline_;}

  private:
    JDevice& device_app;
    VkPipeline computePipeline_;

    void createComputePipeline(const VkPipelineLayout pipelineLayout, 
                               const JShaderModule& shaderModule);

    uint32_t numSamples = 1024;

};



//   // 新文件：Engine/VulkanCore/computePipeline.hpp
//   class JComputePipeline {
//   public:
//       JComputePipeline(JDevice& device, const VkPipelineLayout pipelineLayout,
//                        const std::string& shaderPath,
//                        const VkSpecializationInfo* specInfo = nullptr);
//       ~JComputePipeline();

//       VkPipeline getPipeline() const { return computePipeline_; }

//       // 辅助方法
//       void dispatch(VkCommandBuffer cmd, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ = 1);

//   private:
//       JDevice& device_;
//       VkPipeline computePipeline_;

//       void createComputePipeline(const VkPipelineLayout pipelineLayout, 
//                                 const std::string& shaderPath,
//                                 const VkSpecializationInfo* specInfo);
//   };





// 关于buffer reference// 1. C++端创建缓冲区

// VkBuffer buffer;
// vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);

// // 获取缓冲区的GPU地址
// VkBufferDeviceAddressInfo addressInfo = {
//     .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
//     .buffer = buffer
// };
// VkDeviceAddress address = vkGetBufferDeviceAddress(device,
// &addressInfo);

// 2. 传递地址给着色器

// struct PushConstants {
//     uint64_t dataAddress;  // 缓冲区地址
// };

// PushConstants pc = { .dataAddress = address };
// vkCmdPushConstants(..., &pc);

// 3. 着色器中使用

// layout(push_constant) uniform PC {
//     Data dataPtr;  // 接收地址
// };

// void main() {
//     dataPtr.floats[index] = someValue;  // 直接通过地址写入
// }

// 总结： 这是现代 Vulkan
// 的无绑定技术，通过GPU地址直接访问缓冲区，比传统描述符更灵活高效。











// ● 是的！你这里写的是BRDF LUT（Look-Up Table）生成的计算着色器。现在你需要做的就是：

//   1. 创建计算管线来运行这个着色器

//   需要实现：
//   - ComputePipeline类 - 管理计算管线
//   - 计算着色器编译 - 将.comp编译为.spv
//   - 输出缓冲区 - 存储BRDF LUT结果

//   2. BRDF LUT的作用

//   你的着色器会生成一个2D纹理，其中：
//   - X轴: NdotV (法线与视线夹角)
//   - Y轴: roughness (表面粗糙度)
//   - RG通道: 预计算的BRDF积分结果

//   3. 工作流程

//   启动程序 → 运行BRDF LUT计算着色器 → 生成2D纹理 → 保存/使用于PBR渲染

//   4. 需要的组件

//   1. 计算管线系统
//   2. 缓冲区管理 (你用了buffer_reference)
//   3. 纹理输出 (将buffer转为texture)

//   具体步骤建议：

//   1. 首先: 创建基本的计算管线类
//   2. 然后: 编译着色器并测试基本执行
//   3. 最后: 处理输出并转换为纹理

//   这个BRDF LUT是PBR渲染的重要组件，用于实时高质量的环境光照计算。你想从哪个部分开始实现？

// > compute pipeline是怎么创建的?和创建graphic pipeline区别是什么?

// ● Read(Engine/VulkanCore/pipeline.hpp)
//   ⎿  Read 222 lines (ctrl+r to expand)

// ● Read(Engine/VulkanCore/pipeline.cpp)
//   ⎿  Read 248 lines (ctrl+r to expand)

// ● 根据你的代码和注释，我来解释Compute Pipeline与Graphics Pipeline的区别：

//   Graphics Pipeline vs Compute Pipeline

//   Graphics Pipeline (你现有的)

//   // 需要很多配置：顶点输入、光栅化、混合等
//   VkGraphicsPipelineCreateInfo pipelineInfo{};
//   pipelineInfo.stageCount = configInfo.stageCount;        // 多个着色器阶段
//   pipelineInfo.pVertexInputState = &configInfo.vertexInputStateInfo_;
//   pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
//   pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
//   // ... 等很多状态配置

//   Compute Pipeline (简单得多)

//   // 只需要一个compute shader和pipeline layout
//   VkComputePipelineCreateInfo pipelineInfo{};
//   pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
//   pipelineInfo.stage = {
//       .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//       .stage = VK_SHADER_STAGE_COMPUTE_BIT,
//       .module = computeShaderModule,
//       .pName = "main",
//       .pSpecializationInfo = &specInfo  // 用于设置NUM_SAMPLES等常量
//   };
//   pipelineInfo.layout = pipelineLayout;

//   vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline);



