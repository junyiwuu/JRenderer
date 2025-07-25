#include "PBRmaterial.hpp"
#include "../descriptor/descriptorAllocator.hpp"
#include "load_texture.hpp"
#include "../device.hpp"




JPBRMaterial::JPBRMaterial(
            JDevice& device, 
            std::shared_ptr<JDescriptorAllocator> descriptorAllocator,
            VkDescriptorSetLayout descriptorSetLayout    
    ):
    device_app(device), descriptorAllocator(descriptorAllocator)


{
    //when create a new material (initialize a material)->allocate a desriptor set
    matDescriptorSet_ = descriptorAllocator->allocateDescriptorSet(descriptorSetLayout);
    // matDescriptorSets_.push_back(matDescriptorSet_);

}


JPBRMaterial::~JPBRMaterial(){

    

}



void JPBRMaterial::setAlbedoTexture(const std::shared_ptr<JTexture>& albedo_map){


    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;    
    descriptorWrite.pImageInfo = &albedo_map->getDescriptorImageInfo();
    descriptorWrite.dstSet = matDescriptorSet_;

    descriptorWrites_.push_back(descriptorWrite);
}



// void setRoughnessTexture(const std::shared_ptr<JTexture>& roughness_map){  }


void JPBRMaterial::build(){
    // update this descriptor set
    vkUpdateDescriptorSets(device_app.device(), descriptorWrites_.size(), descriptorWrites_.data(), 0, nullptr);

}


void JPBRMaterial::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout){


    VkDescriptorSet asset_bind[1] = {
        matDescriptorSet_,
    };

    vkCmdBindDescriptorSets(commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pipelineLayout,
                1, /* firstSet */
                1, /* descriptorSetCount */
                asset_bind, /* *pDescriptorSets */
                0, 
                nullptr );


}























































