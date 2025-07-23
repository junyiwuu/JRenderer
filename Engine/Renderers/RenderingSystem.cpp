#include "RenderingSystem.hpp"
#include "../VulkanCore/pipeline.hpp"
#include "../VulkanCore/descriptor.hpp"
#include "../VulkanCore/buffer.hpp"
#include "../VulkanCore/load_texture.hpp"
#include "../VulkanCore/load_model.hpp"
#include "../VulkanCore/structs/uniforms.hpp"
#include "../VulkanCore/structs/pushConstants.hpp"

RenderingSystem::RenderingSystem(JDevice& device, const JSwapchain& swapchain):
    device_app(device), swapchain_app(swapchain)
{


    vikingTexture_obj = std::make_unique<JTexture>("../assets/viking_room.png", device_app);
    vikingModel_obj = JModel::loadModelFromFile(device_app, "../assets/viking_room.obj");
    createDescriptorResources();
    createPipelineResources();

}


RenderingSystem::~RenderingSystem(){


}




 
void RenderingSystem::createDescriptorResources(){


    descriptorSetLayout_glob = JDescriptorSetLayout::Builder{device_app}
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)
        .build();

    descriptorSetLayout_asset = JDescriptorSetLayout::Builder{device_app}
    .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
    .build();


    descriptorPool_obj  = JDescriptorPool::Builder{device_app}
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
        .setMaxSets(4)
        .build();


    descriptorSets_glob.reserve(Global::MAX_FRAMES_IN_FLIGHT);
    //assign ubo descriptor set
    uniformBuffer_objs.reserve(Global::MAX_FRAMES_IN_FLIGHT);
    JDescriptorWriter writer_glob{*descriptorSetLayout_glob, *descriptorPool_obj };  
    for(size_t i =0; i< Global::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        //create uniform buffer
        auto buffer = std::make_unique<JBuffer>(
            device_app,
            sizeof(UniformBufferObject),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        buffer->map();
        uniformBuffer_objs.emplace_back( std::move(buffer) );

        auto& ubo = *uniformBuffer_objs.back();
        auto bufferInfo = ubo.descriptorInfo();
        VkDescriptorSet desSet;
        if( !writer_glob
                    .writeBuffer(0, &bufferInfo)
                    .build(desSet)){ throw std::runtime_error("failed to allocate descriptor set!");    }  

        descriptorSets_glob.push_back(desSet);
    }

    descriptorSets_asset.reserve(1);
    // assign asset descriptor set
    JDescriptorWriter writer_asset{*descriptorSetLayout_asset, *descriptorPool_obj };
    auto imageInfo = vikingTexture_obj->descriptorInfo();
    VkDescriptorSet desSet;
    if(!writer_asset.writeImage(1, &imageInfo).build(desSet)){
        throw std::runtime_error("failed to allocate descriptor set for asset "); }
    descriptorSets_asset.push_back(desSet);

    
}



void RenderingSystem::createPipelineResources(){
    PipelineConfigInfo pipelineConfig{};
    JPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipelineConfig.multisampleInfo.rasterizationSamples = device_app.msaaSamples();

    //for set up push constant
    VkPushConstantRange pushConstanRange{};
    pushConstanRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstanRange.offset = 0;
    pushConstanRange.size = sizeof(pushConstantStruct);

    VkDescriptorSetLayout setLayouts[] = {
                descriptorSetLayout_glob->descriptorSetLayout(), 
                descriptorSetLayout_asset->descriptorSetLayout()};
    pipelinelayout_app = JPipelineLayout::Builder{device_app}
                        .setDescriptorSetLayout(2, setLayouts)
                        .setPushConstRanges(1, &pushConstanRange)
                        .build();

    pipeline_app = std::make_unique<JPipeline>(device_app, swapchain_app,
                    "../shaders/shader.vert.spv", "../shaders/shader.frag.spv",
                    pipelinelayout_app->getPipelineLayout(), pipelineConfig);


}


//including binding descriptor sets, vertex, and pipeline
void RenderingSystem::render(VkCommandBuffer commandBuffer, uint32_t currentFrame){

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_app->getGraphicPipeline());
  
    vikingModel_obj->bind(commandBuffer); //bind vertex buffer and index buffer

    // for (int i = 0; i < numAssets; ++i) {
    //     // 先把本帧的全局 UBO 和 第 i 个 asset 的贴图 绑定到 set 0/1
    //     VkDescriptorSet setsToBind[2] = {
    //         globalSet,               // layout(set=0,binding=0)
    //         descriptorSets_asset[i]  // layout(set=1,binding=1)
    //     };


    VkDescriptorSet setToBind[2] = {
        descriptorSets_glob[currentFrame],
        descriptorSets_asset[0],  // [asset id]
    };

    vkCmdBindDescriptorSets(commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pipelinelayout_app->getPipelineLayout(),
                0,
                2,
                setToBind, 
                0, 
                nullptr );

    for (int j = 0; j<2; j++ )
    {
        pushConstantStruct pushData{};
        pushData.offset = {0.0f, -1.0f + j*1.2, 0.0f};
        pushData.color = {0.0f, 0.0f, 0.2f * j};

        vkCmdPushConstants(commandBuffer, pipelinelayout_app->getPipelineLayout(), 
            VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0, 
            sizeof(pushConstantStruct), &pushData );

    vikingModel_obj->draw(commandBuffer);

}

}











































