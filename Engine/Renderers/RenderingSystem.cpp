#include "RenderingSystem.hpp"
#include "../VulkanCore/pipeline.hpp"
#include "../VulkanCore/descriptor/descriptor.hpp"
#include "../VulkanCore/descriptor/descriptorAllocator.hpp"
#include "../VulkanCore/buffer.hpp"
#include "../VulkanCore/material/load_texture.hpp"
#include "../VulkanCore/material/PBRmaterial.hpp"
#include "../VulkanCore/load_model.hpp"
#include "../VulkanCore/structs/uniforms.hpp"
#include "../VulkanCore/structs/pushConstants.hpp"


RenderingSystem::RenderingSystem(JDevice& device, const JSwapchain& swapchain):
    device_app(device), swapchain_app(swapchain)
{
    createDescriptorResources();
    createPipelineResources();
    loadAssets();
}


RenderingSystem::~RenderingSystem(){


}




 
void RenderingSystem::createDescriptorResources(){

    //create poolsize
    std::vector<VkDescriptorPoolSize> poolSizes = 
    {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,             3},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,     1},
    };

    // create descriptor allocator (create descriptor pool)
    descriptorAllocator_obj = std::make_shared<JDescriptorAllocator>(device_app, poolSizes, 10);


    //create descriptor set layout
    descriptorSetLayout_glob = JDescriptorSetLayout::Builder{device_app}
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)
        .build();

    descriptorSetLayout_asset = JDescriptorSetLayout::Builder{device_app}
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
        .build();

    

    descriptorSets_glob.reserve(Global::MAX_FRAMES_IN_FLIGHT);
    //assign ubo descriptor set
    uniformBuffer_objs.reserve(Global::MAX_FRAMES_IN_FLIGHT);
    JDescriptorWriter writer_glob{*descriptorSetLayout_glob, descriptorAllocator_obj->getDescriptorPool() };  
    for(size_t i =0; i< Global::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        //create uniform buffer
        auto buffer = std::make_unique<JBuffer>(
            device_app,
            sizeof(GlobalUbo),
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
    pushConstanRange.size = sizeof(pushTransformation);

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
void RenderingSystem::render(VkCommandBuffer commandBuffer, 
                                uint32_t currentFrame ){

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_app->getGraphicPipeline());
  
    

    // for (int i = 0; i < numAssets; ++i) {
    //     // 先把本帧的全局 UBO 和 第 i 个 asset 的贴图 绑定到 set 0/1
    //     VkDescriptorSet setsToBind[2] = {
    //         globalSet,               // layout(set=0,binding=0)
    //         descriptorSets_asset[i]  // layout(set=1,binding=1)
    //     };


    // VkDescriptorSet setToBind[2] = {
    //     descriptorSets_glob[currentFrame],
    //     descriptorSets_asset[0],  // [asset id]
    // };

    // vkCmdBindDescriptorSets(commandBuffer, 
    //             VK_PIPELINE_BIND_POINT_GRAPHICS, 
    //             pipelinelayout_app->getPipelineLayout(),
    //             0,
    //             2,
    //             setToBind, 
    //             0, 
    //             nullptr );





    VkDescriptorSet glob_bind[1] = {
        descriptorSets_glob[currentFrame]
    };

    vkCmdBindDescriptorSets(commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pipelinelayout_app->getPipelineLayout(),
                0,
                1,
                glob_bind, 
                0, 
                nullptr );



    // loop all collected assets, and all bind, also aplied push constant
    for (auto& asset : sceneInfo.assets )
    {   
        auto& obj = asset.second;
        if (obj.model == nullptr) { continue;}

        pushTransformation transformPushData{};
        transformPushData.modelMatrix = obj.transform.mat4();

        vkCmdPushConstants(commandBuffer, pipelinelayout_app->getPipelineLayout(), 
            VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0, 
            sizeof(pushTransformation), &transformPushData );


        // VkDescriptorSet asset_bind[1] = {
        //     descriptorSets_asset[0],
        // };
    
        // vkCmdBindDescriptorSets(commandBuffer, 
        //             VK_PIPELINE_BIND_POINT_GRAPHICS, 
        //             pipelinelayout_app->getPipelineLayout(),
        //             1, /* firstSet */
        //             1, /* descriptorSetCount */
        //             asset_bind, /* *pDescriptorSets */
        //             0, 
        //             nullptr );

        obj.material->bind(commandBuffer, pipelinelayout_app->getPipelineLayout());
            
        obj.model->bind(commandBuffer); //bind vertex buffer and index buffer
        obj.model->draw(commandBuffer);

}

}





void RenderingSystem::loadAssets(){

    std::shared_ptr<JModel> viking_model = JModel::loadModelFromFile(device_app, "../assets/viking_room.obj");
    models_["viking_room"] = viking_model;

    std::shared_ptr<JTexture> viking_texture = std::make_shared<JTexture>("../assets/viking_room.png", device_app);
    textures_["viking_room"] = viking_texture;

    std::shared_ptr<JPBRMaterial> pbrMat = std::make_shared<JPBRMaterial>(device_app, 
                                                                        descriptorAllocator_obj, 
                                                                        descriptorSetLayout_asset->descriptorSetLayout());
    pbrMat->setAlbedoTexture(viking_texture);
    pbrMat->build();
    materials_["viking_room_mat"] = pbrMat;
    

    auto vikingHouse = Scene::JAsset::createAsset();
    vikingHouse.model = models_["viking_room"];
    vikingHouse.material = materials_["viking_room_mat"];
    vikingHouse.transform.translation = {0.f, 0.f, 0.f};
    vikingHouse.transform.scale = {1.f, 1.f, 1.f};
    vikingHouse.transform.rotation = {-1.0f, 0.f, 0.0f};
    sceneAssets.emplace(vikingHouse.getId(), std::move(vikingHouse));




}






































