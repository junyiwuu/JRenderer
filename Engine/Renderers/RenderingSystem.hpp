#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>
#include "../VulkanCore/global.hpp"
#include "../Scene/info.hpp"

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

class RenderingSystem{



public:


    RenderingSystem(JDevice& device, const JSwapchain& swapchain);
    ~RenderingSystem();

    NO_COPY(RenderingSystem);

    //commands
    void render(VkCommandBuffer commandBuffer, 
                uint32_t currentFrame, 
                SceneInfo& sceneInfo);

    //getter
    std::vector<std::unique_ptr<JBuffer>>& getUniformBufferObjs() {return uniformBuffer_objs;}



private:
    JDevice& device_app;
    const JSwapchain& swapchain_app;
    //pipeline
    std::unique_ptr<JPipeline> pipeline_app;
    std::unique_ptr<JPipelineLayout> pipelinelayout_app;


    //descriptor
    std::unique_ptr<JDescriptorPool> descriptorPool_obj;
    std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_glob;
    std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_asset;

    std::vector<VkDescriptorSet> descriptorSets_glob;
    std::vector<VkDescriptorSet> descriptorSets_asset;

    std::vector<std::unique_ptr<JBuffer>> uniformBuffer_objs;

    //texture
    std::unique_ptr<JTexture> vikingTexture_obj;
    std::unique_ptr<JModel> vikingModel_obj;

    void createDescriptorResources();
    void createPipelineResources();
    





















};

