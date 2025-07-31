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
    // loadEnvMaps();  // Re-enabled with minimal version
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
    /*  global (change every frame)
        0: uniform buffer  */
    descriptorSetLayout_glob = JDescriptorSetLayout::Builder{device_app}
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)    
        .build();

    /*  global (static)
        0: cubemap  */
    descriptorSetLayout_glob_static = JDescriptorSetLayout::Builder{device_app}
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) 
        .build();

    /*  PBR material
        0: albedo  */
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

    //
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
                descriptorSetLayout_glob_static->descriptorSetLayout(),
                descriptorSetLayout_asset->descriptorSetLayout()};

    pipelinelayout_app = JPipelineLayout::Builder{device_app}
                        .setDescriptorSetLayout(3, setLayouts)
                        .setPushConstRanges(1, &pushConstanRange)
                        .build();

    pipeline_app = std::make_unique<JPipeline>(device_app, swapchain_app,
                    "../shaders/shader.vert.spv", "../shaders/shader.frag.spv",
                    pipelinelayout_app->getPipelineLayout(), pipelineConfig);



    //skybox pipeline
    PipelineConfigInfo pipelineConfig_skybox{};
    JPipeline::defaultPipelineConfigInfo(pipelineConfig_skybox);
    pipelineConfig_skybox.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;  // Disable culling completely
    pipelineConfig_skybox.depthStencilInfo.depthWriteEnable = VK_FALSE;  // Don't write to depth buffer
    pipelineConfig_skybox.depthStencilInfo.depthTestEnable = VK_FALSE;  // Disable depth test completely for debugging
    pipelineConfig_skybox.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;  // Allow drawing at max depth
    pipelineConfig_skybox.multisampleInfo.rasterizationSamples = device_app.msaaSamples();

    pipeline_skybox_app = std::make_unique<JPipeline>(device_app, swapchain_app,
                    "../shaders/skybox.vert.spv", "../shaders/skybox.frag.spv",
                    pipelinelayout_app->getPipelineLayout(), pipelineConfig_skybox);



}


//including binding descriptor sets, vertex, and pipeline
void RenderingSystem::render(VkCommandBuffer commandBuffer, 
                                uint32_t currentFrame ){

    // // Render skybox FIRST (for debugging render order)
    // vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_skybox_app->getGraphicPipeline());

    // // Bind global descriptors for skybox (UBO)
    // VkDescriptorSet glob_bind_skybox_first[1] = {
    //     descriptorSets_glob[currentFrame]
    // };

    // vkCmdBindDescriptorSets(commandBuffer, 
    //             VK_PIPELINE_BIND_POINT_GRAPHICS, 
    //             pipelinelayout_app->getPipelineLayout(),
    //             0,/* firstSet */
    //             1, /* descriptorSetCount */
    //             glob_bind_skybox_first, /* *pDescriptorSets */
    //             0, 
    //             nullptr );

    // // Bind static descriptors (cubemap)
    // if (!descriptorSets_glob_static.empty()) {
    //     VkDescriptorSet glob_static_bind_first[1] = {
    //         descriptorSets_glob_static[0]
    //     };

    //     vkCmdBindDescriptorSets(commandBuffer, 
    //                 VK_PIPELINE_BIND_POINT_GRAPHICS, 
    //                 pipelinelayout_app->getPipelineLayout(),
    //                 1,/* firstSet */
    //                 1, /* descriptorSetCount */
    //                 glob_static_bind_first, /* *pDescriptorSets */
    //                 0, 
    //                 nullptr );

    //     // Simple skybox draw
    //     pushTransformation transformPushData{};
    //     transformPushData.modelMatrix = glm::mat4(1.0f); // Identity matrix for skybox

    //     vkCmdPushConstants(commandBuffer, pipelinelayout_app->getPipelineLayout(), 
    //         VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0, 
    //         sizeof(pushTransformation), &transformPushData );

    //     std::cout << "DEBUG: Drawing skybox FIRST with texture" << std::endl;
    //     vkCmdDraw(commandBuffer, 36, 1, 0, 0);
    // }

    // Now render regular objects
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_app->getGraphicPipeline());

    //global dynamic descriptors
    VkDescriptorSet glob_bind[1] = {
        descriptorSets_glob[currentFrame]
    };

    vkCmdBindDescriptorSets(commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pipelinelayout_app->getPipelineLayout(),
                0,/* firstSet */
                1, /* descriptorSetCount */
                glob_bind, /* *pDescriptorSets */
                0, 
                nullptr );

    // loop all collected assets, and all bind, also aplied push constant
    for (auto& asset : sceneAssets )
    {   
        auto& obj = asset.second;
        if (obj.model == nullptr) { continue;}

        pushTransformation transformPushData{};
        transformPushData.modelMatrix = obj.transform.mat4();

        vkCmdPushConstants(commandBuffer, pipelinelayout_app->getPipelineLayout(), 
            VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0, 
            sizeof(pushTransformation), &transformPushData );

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

    //allocate descriptor automatically in material class
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
    vikingHouse.transform.rotation = {-glm::radians(90.f), 0.f, 0.0f};
    sceneAssets.emplace(vikingHouse.getId(), std::move(vikingHouse));

}


void RenderingSystem::loadEnvMaps(){
    std::shared_ptr<JCubemap> skybox_texture = std::make_shared<JCubemap>("../assets/piazza_bologni_1k.hdr", device_app);
    textures_["skybox"] = skybox_texture;

    JDescriptorWriter writer_glob_static(*descriptorSetLayout_glob_static, descriptorAllocator_obj->getDescriptorPool());
    auto imageInfo = textures_["skybox"]->getDescriptorImageInfo();
    VkDescriptorSet desSet;
    if(!writer_glob_static
                    .writeImage(0, &imageInfo)
                    .build(desSet)){ throw std::runtime_error("failed to allocate descriptor set for cubemap!");}
    descriptorSets_glob_static.push_back(desSet);

    auto skybox = Scene::JEnvMap::createEnvMap();
    skybox.texture = textures_["skybox"];
    skybox.transform.translation = {0.f, 0.f, 0.f};
    skybox.transform.scale = {1.f, 1.f, 1.f};
    skybox.transform.rotation = {-0.0f, 0.f, 0.0f};
    sceneEnvMap.emplace(skybox.getId(), std::move(skybox));




}

































