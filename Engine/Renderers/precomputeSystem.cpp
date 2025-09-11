#include "precomputeSystem.hpp"
#include "../VulkanCore/material/load_texture.hpp"
#include "../VulkanCore/material/cubemapUtils.hpp"
#include "../VulkanCore/material/bitmap.hpp"
#include "../VulkanCore/device.hpp"
#include "../VulkanCore/shaderModule.hpp"
#include "../VulkanCore/pipeline.hpp"
#include "../VulkanCore/commandBuffer.hpp"
#include "../VulkanCore/descriptor/descriptor.hpp"
#include "../VulkanCore/descriptor/descriptorAllocator.hpp"

#include "stb_image.h"
#include "stb_image_write.h"
#include <ktx.h>
#include <vector>

static inline VkDeviceSize Align(VkDeviceSize v, VkDeviceSize a) { return (v + a - 1) & ~(a - 1); }

struct PerFrameData{
    uint32_t face;
    float roughness;

    uint32_t width;
    uint32_t height;

    uint32_t sampleCount;
    uint32_t distribution;         };  

enum Distribution{
    Distribution_Lambertian = 0,
    Distribution_GGX        = 1,   };

PrecomputeSystem::PrecomputeSystem(JDevice& device, const JCubemap& cubemapBase):
    device_app(device)
{
    createComputePipeline();
    generatePrecomputedMaps(cubemapBase);
}


PrecomputeSystem::~PrecomputeSystem(){

}




//first generate these textures, then check how to properly create lighting system
//individual one
void PrecomputeSystem::processCubemap(const JCubemap& cubemapBase, 
            uint32_t distributionIndex ,const char* OUT_ktxPath, uint32_t sampleCount){

    //create empty env container (based on input cubemap)
    TextureConfig prefilterEnvConfig = PrefilterEnvMapConfig(
                                            cubemapBase.getTextureWidth(),
                                            cubemapBase.getTextureHeight(),
                                            VK_FORMAT_R32G32B32A32_SFLOAT);
    //empty container
    prefilterEnvmap_ = std::make_unique<JTextureBase>(device_app, prefilterEnvConfig);

 
    JDescriptorWriter writer(*descriptorSetLayout_app, descriptorAllocator_app->getDescriptorPool());
    //for source env map, just one time set up
    auto srcImageInfo = cubemapBase.getDescriptorImageInfo();
    //use one descriptor set two bindings
    VkDescriptorSet desSet;

    std::vector<VkImageView> loopImageViews;
    //placeholder for initial
    VkImageView placeholder = prefilterEnvmap_->switchViewForMip(1, VK_IMAGE_VIEW_TYPE_2D_ARRAY);
    loopImageViews.push_back(placeholder);  // all need to be destory in the end 


    VkDescriptorImageInfo dstImageInfo{};
    dstImageInfo.imageView = placeholder; //dummy for now
    dstImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;


    if(!writer  .writeImage(0, &srcImageInfo)
                .writeImage(1, &dstImageInfo)
                .build(desSet)){ throw std::runtime_error("failed to allocate descriptor set for cubemap!");}


    //calculate working group size
    uint32_t shader_localX = 16;           // must match the shader
    uint32_t shader_localY = 16;

    // For irradiance (Lambertian), only generate mip 0
    // For prefilter (GGX), generate all mip levels
    uint32_t maxMip = (distributionIndex == 0) ? 1 : prefilterEnvmap_->getMipLevels(); // Lambertian = 0, GGX = 1

    
    for (uint32_t mip = 0; mip < maxMip; mip++) {
        uint32_t faceW = std::max(1u, static_cast<uint32_t>(prefilterEnvmap_->getTextureWidth())  >> mip);
        uint32_t faceH = std::max(1u, static_cast<uint32_t>(prefilterEnvmap_->getTextureHeight()) >> mip);
        uint32_t wkGrp_x = (faceW + shader_localX - 1) / shader_localX;
        uint32_t wkGrp_y = (faceH + shader_localY - 1) / shader_localY;

        JCommandBuffer commandBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        commandBuffer.beginSingleTimeCommands();

        //image view+image layout  
        VkImageView currentMip_imageView = prefilterEnvmap_->switchViewForMip(mip, VK_IMAGE_VIEW_TYPE_2D_ARRAY);
        loopImageViews.push_back(currentMip_imageView);

        //update vkdescriptor set
        VkDescriptorImageInfo updateDstImageInfo{};
        updateDstImageInfo.imageView = currentMip_imageView; 
        updateDstImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        
        VkWriteDescriptorSet overwriteWrite{};
        overwriteWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        overwriteWrite.dstSet = desSet;
        overwriteWrite.dstBinding = 1;
        overwriteWrite.descriptorCount = 1;
        overwriteWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        overwriteWrite.pImageInfo = &updateDstImageInfo;
        vkUpdateDescriptorSets(device_app.device(), 1, &overwriteWrite, 0,nullptr);

        vkCmdBindPipeline(commandBuffer.getCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, prefilterComputePipeline_app->getComputePipeline());
        vkCmdBindDescriptorSets(
                    commandBuffer.getCommandBuffer(), 
                    VK_PIPELINE_BIND_POINT_COMPUTE, 
                    prefilterPipelineLayout_app->getPipelineLayout(),
                    0,/* firstSet */
                    1, /* descriptorSetCount */
                    &desSet, /* *pDescriptorSets */
                    0, 
                    nullptr );
                    
        for (uint32_t face = 0; face < 6; face++) {
            PerFrameData perFrameData{};
            perFrameData.face           = face;
            
            // Set roughness based on distribution type
            if (distributionIndex == 0) { // Lambertian (irradiance)
                perFrameData.roughness = 0.0f; // Maximum roughness for diffuse
            } else { // GGX (prefilter)
                perFrameData.roughness = (float)(mip) / (float)(prefilterEnvmap_->getMipLevels() - 1);
            }
            
            perFrameData.width          = faceW;
            perFrameData.height         = faceH;
            perFrameData.sampleCount    = sampleCount;
            perFrameData.distribution   = distributionIndex;
            
            vkCmdPushConstants(commandBuffer.getCommandBuffer(), prefilterPipelineLayout_app->getPipelineLayout(), 
                    VK_SHADER_STAGE_COMPUTE_BIT, 0, 
                    sizeof(perFrameData), &perFrameData);
            vkCmdDispatch(commandBuffer.getCommandBuffer(), wkGrp_x, wkGrp_y, 1);      

        }
  
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = prefilterEnvmap_->textureImage();
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = mip;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 6;
        barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer.getCommandBuffer(),
                              VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                              VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                            0,0 , nullptr, 0, nullptr, 1, &barrier);


        commandBuffer.endSingleTimeCommands(device_app.graphicsQueue());
    }

    for(VkImageView view:loopImageViews){
        vkDestroyImageView(device_app.device(), view, nullptr);
    }


    //transfer gpu data (image) to buffers;
        std::vector<VkBufferImageCopy> dstBuffers;
        VkDeviceSize totalSize = 0;
        dstBuffers.clear();

        VkDeviceSize offset = 0;

        for(uint32_t mip = 0; mip < maxMip; ++mip){
            uint32_t curWidth  = std::max(1u, static_cast<uint32_t>(prefilterEnvmap_->getTextureWidth()) >> mip);
            uint32_t curHeight = std::max(1u, static_cast<uint32_t>(prefilterEnvmap_->getTextureHeight()) >> mip);
            VkDeviceSize byteThisFace = VkDeviceSize(curWidth) * curHeight * 16; //32bit so use 16

            for(uint32_t face = 0; face < 6; ++face){
                VkBufferImageCopy dstBuf;
                dstBuf.bufferOffset = offset;
                dstBuf.bufferRowLength = 0;
                dstBuf.bufferImageHeight = 0;
                dstBuf.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                dstBuf.imageSubresource.mipLevel = mip;
                dstBuf.imageSubresource.baseArrayLayer = face;
                dstBuf.imageSubresource.layerCount = 1;
                dstBuf.imageOffset = {0,0,0};
                dstBuf.imageExtent = {curWidth, curHeight, 1};
                dstBuffers.push_back(dstBuf);

                offset = Align(offset+byteThisFace, 4); // Vulkan 要求 bufferOffset 至少 4 字节对齐；保持 4 字节对齐最稳

            }
            totalSize = offset;
        }
        
        //staging buffer : on cpu
        JBuffer stagingBuffer(device_app, totalSize, 
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


        // 2. 复制 GPU image 到 staging buffer
        JCommandBuffer copyBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        copyBuffer.beginSingleTimeCommands();
    
        // transition image layout first , Generak->transfer source
        device_app.transitionImageLayout(copyBuffer.getCommandBuffer(),
                prefilterEnvmap_->textureImage(),
                VK_IMAGE_LAYOUT_GENERAL,  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_IMAGE_ASPECT_COLOR_BIT,  maxMip, 6);

        vkCmdCopyImageToBuffer(copyBuffer.getCommandBuffer(),
                prefilterEnvmap_->textureImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                stagingBuffer.buffer(), (uint32_t)dstBuffers.size(), dstBuffers.data());
        
        copyBuffer.endSingleTimeCommands(device_app.graphicsQueue());

        //generate a CPU handle
        stagingBuffer.map();

    

    //create an empty ktx file
    ktxTextureCreateInfo ktxCreateInfo = {
            .vkFormat         = VK_FORMAT_R32G32B32A32_SFLOAT,  
            .baseWidth        = static_cast<ktx_uint32_t>(prefilterEnvmap_->getTextureWidth()),
            .baseHeight       = static_cast<ktx_uint32_t>(prefilterEnvmap_->getTextureHeight()),
            .baseDepth        = 1u,
            .numDimensions    = 2u,
            .numLevels        = maxMip,
            .numLayers        = 1u,
            .numFaces         = 6u,
            .generateMipmaps  = KTX_FALSE,
        };
    ktxTexture2* outKtx = nullptr;
    ktxTexture2_Create(&ktxCreateInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &outKtx);

    //pointer to data
    const uint8_t* src = reinterpret_cast<const uint8_t*>(stagingBuffer.getBufferMapped());
    VkDeviceSize srcOffset = 0;
    //write tto ktx
    for(uint32_t mip = 0; mip < maxMip; ++mip){
        uint32_t curWidth  = std::max(1u, static_cast<uint32_t>(prefilterEnvmap_->getTextureWidth()) >> mip);
        uint32_t curHeight = std::max(1u, static_cast<uint32_t>(prefilterEnvmap_->getTextureHeight()) >> mip);
        VkDeviceSize byteThisFace = VkDeviceSize(curWidth) * curHeight * 16; //32bit so use 16

        for(uint32_t face = 0; face < 6; ++face){
            size_t ktxOffset = 0;
            ktxTexture2_GetImageOffset(outKtx, mip, 0/*layer*/,
                                        face, &ktxOffset );
            std::memcpy(outKtx->pData + ktxOffset, src + srcOffset, size_t(byteThisFace));
            srcOffset = Align(srcOffset + byteThisFace, 4);      }
    }

    stagingBuffer.unmap();

    //write into file
    ktxTexture2_WriteToNamedFile(outKtx, OUT_ktxPath);
    ktxTexture2_Destroy(outKtx);
}




//generate irradiance, prefilter
void PrecomputeSystem::generatePrecomputedMaps(const JCubemap& cubemapBase){
    processCubemap(cubemapBase, 1, "../data/prefilterEnvMap.ktx", 2048);
    processCubemap(cubemapBase, 0, "../data/irradianceMap.ktx", 2048);

}



void PrecomputeSystem::createComputePipeline(){
    //create poolsize
    std::vector<VkDescriptorPoolSize> poolSizes = 
    {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1},
    };

    // create descriptor allocator (create descriptor pool)
    descriptorAllocator_app = std::make_shared<JDescriptorAllocator>(device_app, poolSizes, 2);

    descriptorSetLayout_app = JDescriptorSetLayout::Builder{device_app}
        //sampler+image view+image layout
        .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, 1)
        //image view+image layout
        .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT, 1)
        .build();


    //for prefilter push constant

    VkPushConstantRange prefilter_pushConstantRange{};
    prefilter_pushConstantRange.stageFlags    = VK_SHADER_STAGE_COMPUTE_BIT;
    prefilter_pushConstantRange.offset        = 0;
    prefilter_pushConstantRange.size          = sizeof(PerFrameData); 

    VkDescriptorSetLayout setLayout[] = {descriptorSetLayout_app->descriptorSetLayout()};
    prefilterPipelineLayout_app = JPipelineLayout::Builder{device_app}
                            .setDescriptorSetLayout(1, setLayout)
                            .setPushConstRanges(1, &prefilter_pushConstantRange)
                            .build();


    //compute pipeline -- for prefiltered envmap
    auto code = util::readFile("../shaders/computePrefilIrrad.comp.spv");
    prefilterComputeShader_ = std::make_unique<JShaderModule>(device_app.device(), code);


    prefilterComputePipeline_app = std::make_unique<JComputePipeline>(
                                device_app, *prefilterComputeShader_,
                                prefilterPipelineLayout_app->getPipelineLayout());    


}

























