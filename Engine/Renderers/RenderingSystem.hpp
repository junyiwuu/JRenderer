#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>
#include <filesystem>
#include "../VulkanCore/global.hpp"
#include "../Scene/info.hpp"
#include "../Scene/asset.hpp"


class JPipeline;
class JPipelineLayout;
class JDescriptorPool;
class JDescriptorSetLayout;
class JDescriptorWriter;
class JDevice;
class JTexture;
class JModel;
class JSwapchain;
class JBuffer;
struct SceneInfo;
class JDescriptorAllocator;
class JShaderStages;
class JShaderModule;
class JComputePipeline;
class PrecomputeSystem;
class SamplerManager;
class JCubemap;
class JTexture2D;
class JTextureBase;

namespace UI{
    class UISettings;
}


class RenderingSystem{



public:
    RenderingSystem(JDevice& device, const JSwapchain& swapchain);
    ~RenderingSystem();

    NO_COPY(RenderingSystem);

    //commands
    void render(VkCommandBuffer commandBuffer, 
                uint32_t currentFrame, const UI::UISettings& uiSettings );

    //getter
    std::vector<std::unique_ptr<JBuffer>>& getUniformBufferObjs() {return uniformBuffer_objs;}

    void updateMaterial(const UI::UISettings& uiSettings);

private:
    JDevice& device_app;
    const JSwapchain& swapchain_app;

    //sampler
    std::unique_ptr<SamplerManager> samplerManager_app;
    //pipeline
    std::unique_ptr<JPipeline> pipeline_app;
    std::unique_ptr<JPipeline> pipeline_skybox_app;
    std::unique_ptr<JComputePipeline> brdfComputePipeline_app;

    std::unique_ptr<JPipelineLayout> pipelinelayout_app;
    std::unique_ptr<JPipelineLayout> brdfPipelineLayout_app;
    
    //shader stages - must be kept alive for pipeline lifetime
    std::unique_ptr<JShaderStages> shaderStages_main;
    std::unique_ptr<JShaderStages> shaderStages_skybox;
    std::unique_ptr<JShaderModule> brdfComputeShader;


    //descriptor

    std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_glob;
    std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_glob_static;
    std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_asset;

    std::vector<VkDescriptorSet> descriptorSets_glob;
    std::vector<VkDescriptorSet> descriptorSets_glob_static;

    std::vector<std::unique_ptr<JBuffer>> uniformBuffer_objs;

    void createDescriptorResources();
    void createPipelineResources();
    
    std::shared_ptr<JDescriptorAllocator> descriptorAllocator_obj;

    std::unordered_map<std::string, std::shared_ptr<JModel>>        models_;
    std::unordered_map<std::string, std::shared_ptr<JTexture2D>>    textures_;
    std::unordered_map<std::string, std::shared_ptr<JCubemap>>      cubemaps_; //legacy issue, need to be changed in the future
    std::unordered_map<std::string, std::shared_ptr<JPBRMaterial>>  materials_;


    Scene::JAsset::Map sceneAssets;
    Scene::JEnvMap::Map sceneEnvMap;
    //for brdf lut
    std::unique_ptr<JBuffer> storageBuffer_;
    const uint32_t brdf_w = 256, brdf_h = 256;
    const uint32_t bufferSize = 4u * sizeof(uint16_t) * brdf_h * brdf_w;


    std::unique_ptr<JTextureBase> brdf_lut;
    std::unique_ptr<JTextureBase> prefilterEnvmap;
    std::unique_ptr<JTextureBase> irradianceMap;

    void loadAssets();
    void loadEnvMaps();
    void createBRDFLUT();
    void loadPrecomputedResources();
    void bindGlobalStatic();


    // get the scene info, which including all assets
    SceneInfo sceneInfo{
        sceneAssets,
        sceneEnvMap,
    };


    std::unique_ptr<PrecomputeSystem> precompSystem_app;




    bool lastAlbedoTexState = false;
    bool lastRoughnessTexState = false;
    bool lastMetallicTexState = false;
    bool lastNormalTexState = false;

    std::string lastAlbedoPath;
    std::string lastRoughnessPath;
    std::string lastMetallicPath;
    std::string lastNormalPath;
};

