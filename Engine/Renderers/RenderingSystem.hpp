#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>
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

class RenderingSystem{



public:


    RenderingSystem(JDevice& device, const JSwapchain& swapchain);
    ~RenderingSystem();

    NO_COPY(RenderingSystem);

    //commands
    void render(VkCommandBuffer commandBuffer, 
                uint32_t currentFrame );

    //getter
    std::vector<std::unique_ptr<JBuffer>>& getUniformBufferObjs() {return uniformBuffer_objs;}



private:
    JDevice& device_app;
    const JSwapchain& swapchain_app;
    //pipeline
    std::unique_ptr<JPipeline> pipeline_app;
    std::unique_ptr<JPipelineLayout> pipelinelayout_app;


    //descriptor

    std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_glob;
    std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_asset;

    std::vector<VkDescriptorSet> descriptorSets_glob;
    

    std::vector<std::unique_ptr<JBuffer>> uniformBuffer_objs;



    void createDescriptorResources();
    void createPipelineResources();
    



    std::shared_ptr<JDescriptorAllocator> descriptorAllocator_obj;







    std::unordered_map<std::string, std::shared_ptr<JModel>>        models_;
    std::unordered_map<std::string, std::shared_ptr<JTexture>>      textures_;
    std::unordered_map<std::string, std::shared_ptr<JPBRMaterial>>  materials_;

    Scene::JAsset::Map sceneAssets;

    void loadAssets();

    // get the scene info, which including all assets
    SceneInfo sceneInfo{
        sceneAssets,
    };




};

