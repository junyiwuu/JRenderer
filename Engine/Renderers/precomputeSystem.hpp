//for BRDF, IBL-prefiltered map and irradiance map
// check ktx: convert ktx to exr files  , terminal:  ktx extract prefilterEnvMap.ktx prefilter --all


#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>
#include <algorithm>
#include <optional>
#include "../VulkanCore/global.hpp"

class JCubemap;
class JTextureBase;
class JDevice;
class JShaderModule;
class JPipelineLayout;
class JComputePipeline;
class JDescriptorAllocator;
class JDescriptorSetLayout;



class PrecomputeSystem{

public:
    PrecomputeSystem(JDevice& device, const JCubemap& cubemapBase);
    ~PrecomputeSystem();

    void createComputePipeline();

//optionally create 

    void generatePrecomputedMaps(const JCubemap& cubemapBase);







private:
    JDevice& device_app;

    std::unique_ptr<JTextureBase> prefilterEnvmap_;
    std::unique_ptr<JShaderModule> prefilterComputeShader_;

    std::unique_ptr<JComputePipeline> prefilterComputePipeline_app;
    std::unique_ptr<JPipelineLayout> prefilterPipelineLayout_app;

    std::vector<VkDescriptorSet> descriptorSets_;
    std::shared_ptr<JDescriptorAllocator> descriptorAllocator_app;
    std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_app;

    void processCubemap(const JCubemap& cubemapBase, uint32_t distributionIndex ,const char* OUT_ktxPath, uint32_t sampleCount);





};














