#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>



class JDevice;
class JDescriptorAllocator;
class JTexture2D;
class JSolidColor;
class SamplerManager;


// -----------------------------

/*  0: albedo
    1 : roughness
    2 : metallic
    3 : normal     */
class JPBRMaterial{


public:

    JPBRMaterial(JDevice& device, 
                    std::shared_ptr<JDescriptorAllocator> descriptorAllocator,
                    VkDescriptorSetLayout descriptorSetLayout,
                    SamplerManager& samplerManager);
    ~JPBRMaterial();

    // bind loaded in texture, with, the corresponding pbr set layout, and this material's descriptor set
    void setAlbedoTexture(const JTexture2D& albedo_map);
    void setRoughnessTexture(const JTexture2D& roughness_map);
    void setMetallicTexture(const JTexture2D& metallic_map);
    void setNormalTexture(const JTexture2D& normal_map);

    // build material after loading all textures
    void update();


    // bind with pipeline during command call
    void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

private:
    JDevice&                                    device_app;
    SamplerManager&                             samplerManager;
    std::shared_ptr<JDescriptorAllocator>       descriptorAllocator;
    VkDescriptorSet                             matDescriptorSet_;

    // base color
    std::shared_ptr<JSolidColor> defaultWhite_;
    std::shared_ptr<JSolidColor> defaultBlack_;
    std::shared_ptr<JSolidColor> defaultGrey_;
    std::shared_ptr<JSolidColor> defaultNormal_;
    

    //descriptor writes, will flush in the end
    std::vector<VkWriteDescriptorSet> descriptorWrites_;
    std::vector<VkDescriptorImageInfo> pImageInfos_;

    void initDefault();
};













