#include "PBRmaterial.hpp"
#include "../descriptor/descriptorAllocator.hpp"
#include "load_texture.hpp"
#include "../device.hpp"


//first when initialize, build all default solid color for all channels
//then when call setXXXtexture, overwrite the pimageInfo, 
//one vector for descriptor writes, one vector for pimageinfo, overwrite pimageinfo one

JPBRMaterial::JPBRMaterial(
            JDevice& device, 
            std::shared_ptr<JDescriptorAllocator> descriptorAllocator,
            VkDescriptorSetLayout descriptorSetLayout,
            SamplerManager& samplerManager
    ):
    device_app(device), descriptorAllocator(descriptorAllocator), samplerManager(samplerManager)
{
    defaultWhite_ = std::make_shared<JSolidColor>(device, 1.0f, 1.0f, 1.0f);
    defaultBlack_ = std::make_shared<JSolidColor>(device, 0.0f, 0.0f, 0.0f);
    defaultGrey_  = std::make_shared<JSolidColor>(device, 0.5f, 0.5f, 0.5f);
    // defaultNormal_= std::make_shared<JSolidColor>(device, 1.0f, 1.0f, 1.0f);
    defaultNormal_= std::make_shared<JSolidColor>(device, 0.5f, 0.5f, 1.0f);
    

    //when create a new material (initialize a material)->allocate a desriptor set
    matDescriptorSet_ = descriptorAllocator->allocateDescriptorSet(descriptorSetLayout);
    // matDescriptorSets_.push_back(matDescriptorSet_);
    initDefault();
    update();


}


JPBRMaterial::~JPBRMaterial(){
}


void JPBRMaterial::initDefault(){
    pImageInfos_.clear();
    pImageInfos_.reserve(4); 
    pImageInfos_.push_back(defaultWhite_->getDescriptorImageInfo(samplerManager.getSampler(SamplerType::TextureGlobal)));
    pImageInfos_.push_back(defaultBlack_->getDescriptorImageInfo(samplerManager.getSampler(SamplerType::TextureGlobal)));
    pImageInfos_.push_back(defaultBlack_->getDescriptorImageInfo(samplerManager.getSampler(SamplerType::TextureGlobal)));
    pImageInfos_.push_back(defaultNormal_->getDescriptorImageInfo(samplerManager.getSampler(SamplerType::TextureGlobal)));

    VkDescriptorType desType{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};

    /* 0 : albedo */
    VkWriteDescriptorSet descriptorWrite_Albedo = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = matDescriptorSet_,
        .dstBinding = 0,
        .descriptorCount = 1,
        .descriptorType = desType,
        .pImageInfo = &pImageInfos_[0],
    } ;
    descriptorWrites_.push_back(descriptorWrite_Albedo);

    /* 1 : roughness */
    VkWriteDescriptorSet descriptorWrite_Roughness= {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = matDescriptorSet_,
        .dstBinding = 1,
        .descriptorCount = 1,
        .descriptorType = desType,
        .pImageInfo = &pImageInfos_[1],
    };
    descriptorWrites_.push_back(descriptorWrite_Roughness);

    /* 2 : metallic */
    VkWriteDescriptorSet descriptorWrite_Metallic= {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = matDescriptorSet_,
        .dstBinding = 2,
        .descriptorCount = 1,
        .descriptorType = desType,
        .pImageInfo = &pImageInfos_[2],
    };
    descriptorWrites_.push_back(descriptorWrite_Metallic);


    /* 3: normal */
    VkWriteDescriptorSet descriptorWrite_Normal= {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = matDescriptorSet_,
        .dstBinding = 3,
        .descriptorCount = 1,
        .descriptorType = desType,
        .pImageInfo = &pImageInfos_[3],
    };
    descriptorWrites_.push_back(descriptorWrite_Normal);
}


void JPBRMaterial::update(){
    printf("DEBUG: vkdescriptor update is called \n");
    vkUpdateDescriptorSets(device_app.device(), descriptorWrites_.size(), descriptorWrites_.data(), 0, nullptr);
}

void JPBRMaterial::setAlbedoTexture(const JTexture2D& albedo_map){
    pImageInfos_[0] = albedo_map.getDescriptorImageInfo(samplerManager.getSampler(SamplerType::TextureGlobal));
    update();
}

void JPBRMaterial::setRoughnessTexture(const JTexture2D& roughness_map){
    pImageInfos_[1] = roughness_map.getDescriptorImageInfo(samplerManager.getSampler(SamplerType::TextureGlobal));
    update();
}

void JPBRMaterial::setMetallicTexture(const JTexture2D& metallic_map){
    pImageInfos_[2] = metallic_map.getDescriptorImageInfo(samplerManager.getSampler(SamplerType::TextureGlobal));
    update();
}

void JPBRMaterial::setNormalTexture(const JTexture2D& normal_map){
    printf("DEBUG: set normal is called \n");
    pImageInfos_[3] = normal_map.getDescriptorImageInfo(samplerManager.getSampler(SamplerType::TextureGlobal));
    printf("DEBUG: set normal is calling update \n");
    update();
}




void JPBRMaterial::bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout){


    vkCmdBindDescriptorSets(commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pipelineLayout,
                2, /* set layout index */
                1, /* descriptorSetCount */
                &matDescriptorSet_, /* *pDescriptorSets */
                0, 
                nullptr );
}























































