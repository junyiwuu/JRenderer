#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>



class JDevice;
class JDescriptorAllocator;
class JTexture;


// -----------------------------
class JPBRMaterial{


public:

    JPBRMaterial(JDevice& device, 
                    std::shared_ptr<JDescriptorAllocator> descriptorAllocator,
                    VkDescriptorSetLayout descriptorSetLayout  );
    ~JPBRMaterial();



    // bind loaded in texture, with, the corresponding pbr set layout, and this material's descriptor set
    void setAlbedoTexture(const std::shared_ptr<JTexture>& albedo_map);
    // void setRoughnessTexture(const std::shared_ptr<JTexture>& roughness_map);

    //(albedo 0, roughness 1)

    //set constant value
    void setAlbedoConstant(glm::vec3& color);
    // void setRoughnessConstant(float roughness_value);

    // build material after loading all textures
    void build();


    // bind with pipeline during command call
    void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);





private:

    JDevice&                                    device_app;
    std::shared_ptr<JDescriptorAllocator>       descriptorAllocator;
    VkDescriptorSet                             matDescriptorSet_;
    // std::vector<VkDescriptorSet>                matDescriptorSets_;


    //PBR textures
    std::shared_ptr<JTexture> albedo_map;
    // std::shared_ptr<JTexture> roughness_map;


    //descriptor writes, will flush in the end
    std::vector<VkWriteDescriptorSet> descriptorWrites_;














};













