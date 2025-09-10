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
#include "../VulkanCore/shaderModule.hpp"
#include "../VulkanCore/commandBuffer.hpp"
#include "precomputeSystem.hpp"
#include "../Interface/uiSettings.hpp"

#include "ktx.h"


RenderingSystem::RenderingSystem(JDevice& device, const JSwapchain& swapchain):
    device_app(device), swapchain_app(swapchain)
{
    samplerManager_app = std::make_unique<SamplerManager>(device_app);
    createDescriptorResources();
    createPipelineResources();
    createBRDFLUT();  //need to be moved to precomputeSystem
    loadAssets();
    loadEnvMaps();
    // Use the skybox cubemap as the base environment map for precomputation
    auto* skyboxCubemap = static_cast<JCubemap*>(cubemaps_["skybox"].get());
    precompSystem_app = std::make_unique<PrecomputeSystem>(device_app, *skyboxCubemap);
    loadPrecomputedResources();
    bindGlobalStatic();

}


RenderingSystem::~RenderingSystem(){
    samplerManager_app->destroy();
}




void RenderingSystem::loadPrecomputedResources(){

    /* load in BRDF LUT */
    ktxTexture2* ktxTex_brdf = nullptr;
    if(ktxTexture2_CreateFromNamedFile("../data/BRDF_LUT.ktx", 
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTex_brdf) != KTX_SUCCESS){
        throw std::runtime_error("Failed to load BRDF lUT ktx file");      }
    TextureConfig brdf_config = {
        .format = static_cast<VkFormat>(ktxTex_brdf->vkFormat),
        .channels = ktxTexture2_GetNumComponents(ktxTex_brdf),
        .imageType = VK_IMAGE_TYPE_2D,
        .usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .extent = {ktxTex_brdf->baseWidth, ktxTex_brdf->baseHeight, 1},
        .mipLevels = ktxTex_brdf->numLevels,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .arrayLayers = 1,
    };
    brdf_lut = std::make_unique<JTextureBase>(device_app, brdf_config);
    TexUtils::UploadKtxToTexture(device_app, ktxTex_brdf, *brdf_lut, /*isCube*/false);
    ktxTexture2_Destroy(ktxTex_brdf);



    /* load in irradiance */
    ktxTexture2* ktxTex_irrad = nullptr;
    if(ktxTexture2_CreateFromNamedFile("../data/irradianceMap.ktx", 
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTex_irrad) != KTX_SUCCESS){
        throw std::runtime_error("ktx file load failed");      }
    const bool irradIsCube = (ktxTex_irrad->numFaces == 6);
    TextureConfig irrad_config = {
        .format = static_cast<VkFormat>(ktxTex_irrad->vkFormat),
        .channels = ktxTexture2_GetNumComponents(ktxTex_irrad),
        .imageType = VK_IMAGE_TYPE_2D,
        .usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .createFlags = irradIsCube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,
        .extent = {ktxTex_irrad->baseWidth, ktxTex_irrad->baseHeight, 1},
        .mipLevels = ktxTex_irrad->numLevels,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .viewType = irradIsCube ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
        .arrayLayers = irradIsCube ? 6u : 1u,
    };
    irradianceMap = std::make_unique<JTextureBase>(device_app, irrad_config);
    TexUtils::UploadKtxToTexture(device_app, ktxTex_irrad, *irradianceMap, true);
    ktxTexture2_Destroy(ktxTex_irrad);
    
         
    /* load in prefilter */
    ktxTexture2* ktxTex_prefilter = nullptr;
    if(ktxTexture2_CreateFromNamedFile("../data/prefilterEnvMap.ktx", 
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTex_prefilter) != KTX_SUCCESS){
        throw std::runtime_error("ktx file load failed");      }
    const bool prefilterIsCube = (ktxTex_prefilter->numFaces == 6);
    TextureConfig prefilter_config = {
        .format = static_cast<VkFormat>(ktxTex_prefilter->vkFormat),
        .channels = ktxTexture2_GetNumComponents(ktxTex_prefilter),
        .imageType = VK_IMAGE_TYPE_2D,
        .usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .createFlags = prefilterIsCube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0u,
        .extent = {ktxTex_prefilter->baseWidth, ktxTex_prefilter->baseHeight, 1},
        .mipLevels = ktxTex_prefilter->numLevels,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .viewType = prefilterIsCube ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D,
        .arrayLayers = prefilterIsCube ? 6u : 1u,
    };
    prefilterEnvmap = std::make_unique<JTextureBase>(device_app, prefilter_config);
    TexUtils::UploadKtxToTexture(device_app, ktxTex_prefilter, *prefilterEnvmap, true);
    ktxTexture2_Destroy(ktxTex_prefilter);

}


void RenderingSystem::createBRDFLUT(){


    storageBuffer_ = std::make_unique<JBuffer>(device_app, bufferSize, 
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT|VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT      );

   //BRDF LUT -----------------------------------------------------------------------------
   pushBRDFStruct brdfPushConstant{};
   brdfPushConstant.BRDF_H = brdf_h;
   brdfPushConstant.BRDF_W = brdf_w;
   brdfPushConstant.bufferAddr = storageBuffer_->getBufferAddress();

   JCommandBuffer commandBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
   commandBuffer.beginSingleTimeCommands();
   
   vkCmdBindPipeline(commandBuffer.getCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, brdfComputePipeline_app->getComputePipeline());
   vkCmdPushConstants(commandBuffer.getCommandBuffer(), brdfPipelineLayout_app->getPipelineLayout(), 
                       VK_SHADER_STAGE_COMPUTE_BIT, 0, 
                       sizeof(brdfPushConstant), &brdfPushConstant);
   vkCmdDispatch(commandBuffer.getCommandBuffer(), (brdf_w / 16), (brdf_h / 16), 1);
   //用graphicqueue来计算compute的东西
   commandBuffer.endSingleTimeCommands(device_app.graphicsQueue());


   //write to ktx file
   
   ktxTextureCreateInfo ktxCreateInfo{};
   ktxCreateInfo.vkFormat         = VK_FORMAT_R16G16B16A16_SFLOAT;
   ktxCreateInfo.baseWidth        = brdf_w;
   ktxCreateInfo.baseHeight       = brdf_h;
   ktxCreateInfo.baseDepth        = 1;
   ktxCreateInfo.numDimensions    = 2;
   ktxCreateInfo.numLayers        = 1;
   ktxCreateInfo.numLevels        = 1;
   ktxCreateInfo.numFaces         = 1;
   ktxCreateInfo.generateMipmaps  = KTX_FALSE;
   ktxCreateInfo.isArray          = KTX_FALSE;
   
   ktxTexture2* lutTexture = nullptr;
   if (ktxTexture2_Create(&ktxCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &lutTexture) != KTX_SUCCESS){
       throw std::runtime_error("BRDF LUT ktx file create failed!");   };
       
    
    JBuffer stagingBuffer(device_app, storageBuffer_->getSize(),   // in gpu but cpu can access
                VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    util::copyBuffer(storageBuffer_->buffer() , stagingBuffer.buffer(),  storageBuffer_->getSize(), 
            device_app.device(), device_app.getCommandPool(), device_app.graphicsQueue()); 

    stagingBuffer.map();    

    const ktx_size_t imageSize = static_cast<ktx_size_t>(brdf_w) * brdf_h * 4 * sizeof(uint16_t);
    if(ktxTexture_SetImageFromMemory(ktxTexture(lutTexture), /*level*/0, /*layer*/0, /*faceSlice*/0,
                            reinterpret_cast<const ktx_uint8_t*>(stagingBuffer.getBufferMapped()), imageSize )
              != KTX_SUCCESS)
    {
        throw std::runtime_error("BRDF LUT ktx texture set image from memory failed!") ;
    };

    ktxTexture2_WriteToNamedFile(lutTexture, "../data/BRDF_LUT.ktx");
    ktxTexture2_Destroy(lutTexture);

    stagingBuffer.unmap();
}



 
void RenderingSystem::createDescriptorResources(){

    //create poolsize
    // Reserve enough descriptors for UBOs and image samplers (materials + IBL)
    std::vector<VkDescriptorPoolSize> poolSizes = 
    {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,             8},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,     32},
    };

    // create descriptor allocator (create descriptor pool)
    descriptorAllocator_obj = std::make_shared<JDescriptorAllocator>(device_app, poolSizes, 10);


    //create descriptor set layout
    /*  global (change every frame)
        0: uniform buffer  */
        //uniform buffer also used in fragment shader, so need to add fragmanet flag
    descriptorSetLayout_glob = JDescriptorSetLayout::Builder{device_app}
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
                    VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 1)    
        .build();

    /*  global (static)
        0: cubemap  */
    descriptorSetLayout_glob_static = JDescriptorSetLayout::Builder{device_app}
        //skybox
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) 
        //brdf lut
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) 
        //irradiance map
        .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) 
        //prefiltered envmap
        .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1) 
        .build();

    /*  PBR material
        0: albedo , 1: roughness, 2: metallic, 3: normal */
    descriptorSetLayout_asset = JDescriptorSetLayout::Builder{device_app}
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
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
    //compute pipeline -- for BRDF LUT
    auto code = util::readFile("../shaders/BRDF_LUT.comp.spv");
    brdfComputeShader = std::make_unique<JShaderModule>(device_app.device(), code);

    VkPushConstantRange brdf_pushConstantRange{};
    brdf_pushConstantRange.stageFlags    = VK_SHADER_STAGE_COMPUTE_BIT;
    brdf_pushConstantRange.offset        = 0;
    brdf_pushConstantRange.size          = sizeof(uint32_t) * 2 + sizeof(uint64_t); //BRDF_W + BRDF_H + buffer address

    brdfPipelineLayout_app = JPipelineLayout::Builder{device_app}
                            .setPushConstRanges(1, &brdf_pushConstantRange)
                            .build();

    brdfComputePipeline_app = std::make_unique<JComputePipeline>(
                                device_app, *brdfComputeShader,
                                brdfPipelineLayout_app->getPipelineLayout());    



    //create shader stage
    shaderStages_main = std::make_unique<JShaderStages>(
        JShaderStages::Builder(device_app)
                        .setVert("../shaders/shader.vert.spv")
                        .setFrag( "../shaders/shader.frag.spv")
                        .build());
    //
    PipelineConfigInfo pipelineConfig{};
    JPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipelineConfig.multisampleInfo.rasterizationSamples = device_app.msaaSamples();
    //input shader stage
    auto& stages = shaderStages_main->getStageInfos();
    pipelineConfig.pStages = stages.data();
    pipelineConfig.stageCount = static_cast<uint32_t>(stages.size());

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescription = Vertex::getAttributeDescriptions();
    pipelineConfig.setVertexInputState(
            std::span{ &bindingDescription, 1}, 
            std::span{attributeDescription}    );

    //set up push constant
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
                    pipelinelayout_app->getPipelineLayout(), pipelineConfig);


    //skybox pipeline
    shaderStages_skybox = std::make_unique<JShaderStages>(
        JShaderStages::Builder(device_app)
        .setVert("../shaders/skybox.vert.spv")
        .setFrag( "../shaders/skybox.frag.spv")
        .build());

    PipelineConfigInfo pipelineConfig_skybox{};
    JPipeline::defaultPipelineConfigInfo(pipelineConfig_skybox);
    pipelineConfig_skybox.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;  // Disable culling completely
    pipelineConfig_skybox.depthStencilInfo.depthWriteEnable = VK_FALSE;  // Don't write to depth buffer
    pipelineConfig_skybox.depthStencilInfo.depthTestEnable = VK_FALSE;  // Disable depth test completely for debugging
    pipelineConfig_skybox.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;  // Allow drawing at max depth
    pipelineConfig_skybox.multisampleInfo.rasterizationSamples = device_app.msaaSamples();
    auto& stages_skybox = shaderStages_skybox->getStageInfos();
    pipelineConfig_skybox.pStages = stages_skybox.data();
    pipelineConfig_skybox.stageCount = static_cast<uint32_t>(stages_skybox.size());
    pipeline_skybox_app = std::make_unique<JPipeline>(device_app, swapchain_app,
                    pipelinelayout_app->getPipelineLayout(), pipelineConfig_skybox);
}


//including binding descriptor sets, vertex, and pipeline
void RenderingSystem::render(VkCommandBuffer commandBuffer, 
                                uint32_t currentFrame, const UI::UISettings& uiSettings ){

    /* --------------------------------
     ------------ skybox ------------
    ----------------------------------*/
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_skybox_app->getGraphicPipeline());
    VkDescriptorSet glob_bind_forSkybox[1] = {
        descriptorSets_glob[currentFrame]
    };

    vkCmdBindDescriptorSets(commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pipelinelayout_app->getPipelineLayout(),
                0,/* firstSet */
                1, /* descriptorSetCount */
                glob_bind_forSkybox, /* *pDescriptorSets */
                0, 
                nullptr );
    // both need camera uniform buffer for both pipelines

    // Bind static descriptors (cubemap)
    if (!descriptorSets_glob_static.empty()) {
        VkDescriptorSet glob_static_bind[1] = {
            descriptorSets_glob_static[0]
        };

        vkCmdBindDescriptorSets(commandBuffer, 
                    VK_PIPELINE_BIND_POINT_GRAPHICS, 
                    pipelinelayout_app->getPipelineLayout(),
                    1,/* firstSet */
                    1, /* descriptorSetCount */
                    glob_static_bind, /* *pDescriptorSets */
                    0, 
                    nullptr );

        // Simple skybox draw
        pushTransformation transformPushData{};
        transformPushData.modelMatrix = glm::mat4(1.0f); // Identity matrix for skybox

        vkCmdPushConstants(commandBuffer, pipelinelayout_app->getPipelineLayout(), 
            VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0, 
            sizeof(pushTransformation), &transformPushData );

        vkCmdDraw(commandBuffer, 36, 1, 0, 0);
    }

    /* --------------------------------
     --- Now render regular objects ---
    ----------------------------------*/
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_app->getGraphicPipeline());

    // Bind global dynamic descriptors (camera UBO)
    VkDescriptorSet glob_bind_forAssets[1] = {
        descriptorSets_glob[currentFrame]
    };

    vkCmdBindDescriptorSets(commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pipelinelayout_app->getPipelineLayout(),
                0,/* firstSet */
                1, /* descriptorSetCount */
                glob_bind_forAssets, /* *pDescriptorSets */
                0, 
                nullptr );

    // Bind global static descriptors (IBL textures: BRDF, irradiance, prefilter)
    if (!descriptorSets_glob_static.empty()) {
        VkDescriptorSet glob_static_bind_assets[1] = {
            descriptorSets_glob_static[0]
        };

        vkCmdBindDescriptorSets(commandBuffer, 
                    VK_PIPELINE_BIND_POINT_GRAPHICS, 
                    pipelinelayout_app->getPipelineLayout(),
                    1,/* firstSet */
                    1, /* descriptorSetCount */
                    glob_static_bind_assets, /* *pDescriptorSets */
                    0, 
                    nullptr );
    }

    // loop all collected assets, and all bind, also aplied push constant
    for (auto& asset : sceneAssets )
    {   
        auto& obj = asset.second;
        if (obj.model == nullptr) { continue;}

        pushTransformation transformPushData{};
        transformPushData.modelMatrix = obj.transform.mat4();
        transformPushData.baseColor = glm::vec3(uiSettings.baseColor[0],
            uiSettings.baseColor[1],
            uiSettings.baseColor[2]);
        transformPushData.roughness = uiSettings.roughness;
        transformPushData.metallic = uiSettings.metallic;
        // Set the flags
        transformPushData.inputAlbedoPath = uiSettings.inputAlbedoPath ? 1 : 0;
        transformPushData.inputRoughnessPath = uiSettings.inputRoughnessPath ? 1 : 0;
        transformPushData.inputMetallicPath = uiSettings.inputMetallicPath ? 1 : 0;
        transformPushData.inputNormalPath = uiSettings.inputNormalPath ? 1 : 0;

        vkCmdPushConstants(commandBuffer, pipelinelayout_app->getPipelineLayout(), 
            VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0, 
            sizeof(pushTransformation), &transformPushData );

        obj.material->bind(commandBuffer, pipelinelayout_app->getPipelineLayout());
        obj.model->bind(commandBuffer); //bind vertex buffer and index buffer
        obj.model->draw(commandBuffer); 
    }
}




void RenderingSystem::loadAssets(){

    // std::shared_ptr<JModel> fruit_model = JModel::loadModelFromFile(device_app, "/mnt/D/material_support/3d_industrial_vj2oefbs/vj2oefbs_lod0.fbx");
    // std::shared_ptr<JModel> fruit_model = JModel::loadModelFromFile(device_app, "../assets/Cerberus/Cerberus_LP.FBX");
    std::shared_ptr<JModel> fruit_model = JModel::loadModelFromFile(device_app, "../assets/sphere_highres.obj");
    models_["pomoFruit"] = fruit_model;

    std::shared_ptr<JTexture2D> fruit_albedo = std::make_shared<JTexture2D>(device_app, "../assets/Cerberus/Cerberus_A.tga", VK_FORMAT_R8G8B8A8_SRGB);
    textures_["pomoFruit_Albedo"] = fruit_albedo;
    std::shared_ptr<JTexture2D> fruit_rough = std::make_shared<JTexture2D>(device_app, "../assets/Cerberus/Cerberus_R.tga", VK_FORMAT_R8G8B8A8_UNORM);  //has to be 4 channels
    textures_["pomoFruit_Roughness"] = fruit_rough;
    std::shared_ptr<JTexture2D> fruit_metallic = std::make_shared<JTexture2D>(device_app, "../assets/Cerberus/Cerberus_M.tga", VK_FORMAT_R8G8B8A8_UNORM);
    textures_["pomoFruit_Metallic"] = fruit_metallic;
    std::shared_ptr<JTexture2D> fruit_normal = std::make_shared<JTexture2D>(device_app, "../assets/Cerberus/Cerberus_N.tga", VK_FORMAT_R8G8B8A8_UNORM);
    textures_["pomoFruit_Normal"] = fruit_normal;


    //allocate descriptor automatically in material class
    std::shared_ptr<JPBRMaterial> pbrMat = std::make_shared<JPBRMaterial>(device_app, 
                                                                        descriptorAllocator_obj, 
                                                                        descriptorSetLayout_asset->descriptorSetLayout(),
                                                                        *samplerManager_app);
    pbrMat->setAlbedoTexture(*fruit_albedo);
    pbrMat->setRoughnessTexture(*fruit_rough);
    pbrMat->setNormalTexture(*fruit_normal);
    pbrMat->setMetallicTexture(*fruit_metallic);
    materials_["pomoFruit_mat"] = pbrMat;
    
    auto pomoFruit = Scene::JAsset::createAsset();
    pomoFruit.model = models_["pomoFruit"];
    pomoFruit.material = materials_["pomoFruit_mat"];
    pomoFruit.transform.translation = {0.f, 0.f, 0.f};
    pomoFruit.transform.scale = {0.1f, 0.1f, 0.1f};
    pomoFruit.transform.rotation = {-glm::radians(90.f), 0.f, 0.0f};
    sceneAssets.emplace(pomoFruit.getId(), std::move(pomoFruit));
}


void RenderingSystem::loadEnvMaps(){
    std::shared_ptr<JCubemap> skybox_texture = std::make_shared<JCubemap>("../assets/german_town_street_2k.hdr", device_app);
    cubemaps_["skybox"] = skybox_texture;



    auto skybox = Scene::JEnvMap::createEnvMap();
    skybox.texture = cubemaps_["skybox"];
    skybox.transform.translation = {0.f, 0.f, 0.f};
    skybox.transform.scale = {1.f, 1.f, 1.f};
    skybox.transform.rotation = {-0.0f, 0.f, 0.0f};
    sceneEnvMap.emplace(skybox.getId(), std::move(skybox));



}



void RenderingSystem::bindGlobalStatic(){
    /*  0 : skybox
        1 : brdf lut
        2 : irradiance map
        3 : prefilter envmap  */

    JDescriptorWriter writer_glob_static(*descriptorSetLayout_glob_static, descriptorAllocator_obj->getDescriptorPool());

    auto CubemapInfo = cubemaps_["skybox"]->getDescriptorImageInfo();

    //create specific sampler, might merge into sampler manager later
    auto BrdfSamplerInfo = SamplerCreateInfoBuilder()
                    .addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                    .maxLod(0)
                    .getInfo();
    brdf_lut->createCustomSampler(BrdfSamplerInfo);
    irradianceMap->createCustomSampler(BrdfSamplerInfo);
    
    auto PrefilterSamplerInfo = SamplerCreateInfoBuilder()
                    .addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                    .getInfo();
    prefilterEnvmap->createCustomSampler(PrefilterSamplerInfo);

    auto BrdfInfo = brdf_lut->getDesImageInfo();
    auto irradianceInfo =  irradianceMap->getDesImageInfo();    
    auto prefilterInfo =  prefilterEnvmap->getDesImageInfo();
    
    VkDescriptorSet desSet;
    if(!writer_glob_static
                    .writeImage(0, &CubemapInfo)
                    .writeImage(1, &BrdfInfo)
                    .writeImage(2, &irradianceInfo)
                    .writeImage(3, &prefilterInfo)
                    .build(desSet)){ throw std::runtime_error("failed to allocate descriptor set for cubemap!");}
    descriptorSets_glob_static.push_back(desSet);
}






void RenderingSystem::updateMaterial(const UI::UISettings& uiSettings){

    vkDeviceWaitIdle(device_app.device());
    auto& pbrMat = materials_["pomoFruit_mat"];
    bool needsUpdate = false;

    //need to use function to not do this repeat thing


    //when toggle on, and the path is valid, and the path is not empty
    if(uiSettings.inputAlbedoPath && lastAlbedoPath != std::string(uiSettings.albedoTexPath) && strlen(uiSettings.albedoTexPath) >0){
        //check if file exist
        if(!std::filesystem::exists(uiSettings.albedoTexPath)){
            // std::cout << "warning: Albedo texture path not found!: " << uiSettings.albedoTexPath << std::endl;
            return;
        }
        try{
            vkDeviceWaitIdle(device_app.device());
            std::shared_ptr<JTexture2D> fruit_albedo = std::make_shared<JTexture2D>(device_app, uiSettings.albedoTexPath, VK_FORMAT_R8G8B8A8_SRGB);
            textures_["pomoFruit_Albedo"] = fruit_albedo;  //must for render
            pbrMat->setAlbedoTexture(*fruit_albedo);
            lastAlbedoPath = std::string(uiSettings.albedoTexPath);
        }catch(const std::exception& error){
            std::cout << "warning: Failed to load Albedo Texture: " <<uiSettings.albedoTexPath<<std::endl;
        }
    }

    
    if(uiSettings.inputRoughnessPath && lastRoughnessPath != std::string(uiSettings.roughnessTexPath) && strlen(uiSettings.roughnessTexPath) >0){
    //check if file exist
        if(!std::filesystem::exists(uiSettings.roughnessTexPath)){
            // std::cout << "warning: Roughness texture path not found!: " << uiSettings.albedoTexPath << std::endl;
            return;
        }
        try{
            vkDeviceWaitIdle(device_app.device());
            std::shared_ptr<JTexture2D> fruit_roughness = std::make_shared<JTexture2D>(device_app, uiSettings.roughnessTexPath, VK_FORMAT_R8G8B8A8_UNORM);
            pbrMat->setRoughnessTexture(*fruit_roughness);
            textures_["pomoFruit_Roughness"] = fruit_roughness;  //must for render
            lastRoughnessPath = std::string(uiSettings.roughnessTexPath);
        }catch(const std::exception& error){
            std::cout << "warning: Failed to load Roughness Texture: " <<uiSettings.roughnessTexPath<<std::endl;
        }
    }
    
    if(uiSettings.inputMetallicPath && lastMetallicPath != std::string(uiSettings.metallicTexPath) && strlen(uiSettings.metallicTexPath) >0){
        //check if file exist
        if(!std::filesystem::exists(uiSettings.metallicTexPath)){
            // std::cout << "warning: Metallic texture path not found!: " << uiSettings.albedoTexPath << std::endl;
            return;
        }
        try{
            vkDeviceWaitIdle(device_app.device());
            std::shared_ptr<JTexture2D> fruit_Metallic = std::make_shared<JTexture2D>(device_app, uiSettings.metallicTexPath, VK_FORMAT_R8G8B8A8_UNORM);
            pbrMat->setMetallicTexture(*fruit_Metallic);
            textures_["pomoFruit_Metallic"] = fruit_Metallic;  //must for render
            lastMetallicPath = std::string(uiSettings.metallicTexPath);
        }catch(const std::exception& error){
            std::cout << "warning: Failed to load Metallic Texture: " <<uiSettings.metallicTexPath<<std::endl;
            }
        }

        

    if(uiSettings.inputNormalPath && lastNormalPath != std::string(uiSettings.normalTexPath) && strlen(uiSettings.normalTexPath) >0){
        //check if file exist
        if(!std::filesystem::exists(uiSettings.normalTexPath)){
            // std::cout << "warning: Normal texture path not found!: " << uiSettings.albedoTexPath << std::endl;
            return;
        }
        try{
            vkDeviceWaitIdle(device_app.device());
            std::shared_ptr<JTexture2D> fruit_Normal = std::make_shared<JTexture2D>(device_app, uiSettings.normalTexPath, VK_FORMAT_R8G8B8A8_UNORM);
            pbrMat->setNormalTexture(*fruit_Normal);
            textures_["pomoFruit_Normal"] = fruit_Normal;  //must for render
            lastNormalPath = std::string(uiSettings.normalTexPath);
        }catch(const std::exception& error){
            std::cout << "warning: Failed to load Normal Texture: " <<uiSettings.normalTexPath<<std::endl;
            }
        }


}























