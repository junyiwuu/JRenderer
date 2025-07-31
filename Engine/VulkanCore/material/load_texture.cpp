#include "load_texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include "cubemapUtils.hpp"
#include "../buffer.hpp"
#include "../device.hpp"
#include "../commandBuffer.hpp"

JTexture::JTexture(const std::string& path, JDevice& device):
    device_app(device)    
{
    createTextureImage(path, device_app);
    createTextureImageView();
    createTextureSampler();
    createDescriptorInfo();
}


JTexture::~JTexture(){
    vkDestroyImage(device_app.device(), textureImage_, nullptr);
    vkFreeMemory(device_app.device(), textureImageMemory_, nullptr);
    vkDestroySampler(device_app.device(), textureSampler_, nullptr);
    vkDestroyImageView(device_app.device(), textureImageView_, nullptr);

}

void JTexture::createDescriptorInfo(){
    descriptorImageInfo_ = VkDescriptorImageInfo
    {
        textureSampler_,
        textureImageView_,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
}

void JTexture::createTextureImage(const std::string& path, JDevice& device_app) {
    // int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4; // 4 channels, integer, so each channel 1 byte
    mipLevels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight))))+1;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");}


    JBuffer stagingBuffer(device_app, imageSize, 
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* data;
    vkMapMemory(device_app.device(), stagingBuffer.bufferMemory(), 0, stagingBuffer.getSize(), 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device_app.device(), stagingBuffer.bufferMemory());


    stbi_image_free(pixels);

    auto imageInfo = ImageCreateInfoBuilder(texWidth, texHeight)
                    .mipLevels(mipLevels_)
                    .usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_SAMPLED_BIT)
                    .getInfo();
    device_app.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage_, textureImageMemory_);

    JCommandBuffer commandBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    commandBuffer.beginSingleTimeCommands();

    device_app.transitionImageLayout(commandBuffer.getCommandBuffer() ,textureImage_,  
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,   VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT, mipLevels_);

    copyBufferToImage(commandBuffer.getCommandBuffer(), stagingBuffer.buffer(), textureImage_, 
        static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    
    commandBuffer.endSingleTimeCommands(device_app.graphicsQueue());


    // device_app.transitionImageLayout(textureImage_, VK_FORMAT_R8G8B8A8_SRGB, 
    //     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels_);
    generateMipmaps(textureImage_, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels_);

}


void JTexture::createTextureImageView(){
    auto viewInfo = ImageViewCreateInfoBuilder(textureImage_)
                    .mipLevels(0, mipLevels_)
                    .getInfo();
    device_app.createImageViewWithInfo(viewInfo, textureImageView_);

}


void JTexture::copyBufferToImage(VkCommandBuffer commandBuffer,VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) 
{
    // VkCommandBuffer commandBuffer = util::beginSingleTimeCommands(device_app.device(), device_app.getCommandPool());
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    // util::endSingleTimeCommands(device_app.device(), commandBuffer, device_app.getCommandPool(), device_app.graphicsQueue());
}





void JTexture::createTextureSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device_app.physicalDevice(), &properties);

    auto samplerInfo = SamplerCreateInfoBuilder()
                        .maxAnisotropy(properties.limits.maxSamplerAnisotropy)
                        .getInfo();


    if (vkCreateSampler(device_app.device(), &samplerInfo, nullptr, &textureSampler_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}



 void JTexture::generateMipmaps(VkImage image, VkFormat imageFormat, 
                      int32_t texWidth, int32_t texHeight, uint32_t mipLevels){

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(device_app.physicalDevice(), imageFormat, &formatProperties);

    if(!(formatProperties.optimalTilingFeatures&VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)){
        throw std::runtime_error("texture iamge format does not support linear blitting!"); }
    
    VkCommandBuffer commandBuffer = util::beginSingleTimeCommands(device_app.device(), device_app.getCommandPool());
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;  // oldlayout用的是什么布局
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; 
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, 
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier );

    util::endSingleTimeCommands(device_app.device(), commandBuffer, device_app.getCommandPool(), device_app.graphicsQueue());
}

/*  ------------------

     ----Cube Map-----

    ------------------*/
JCubemap::JCubemap(const std::string& path, JDevice& device):
    JTexture(device)
{
    createCubemapImage(path, device_app);
    createCubemapImageView();
    createCubemapSampler();
    createDescriptorInfo();

}



JCubemap::~JCubemap(){
    vkDestroyImage(device_app.device(), textureImage_, nullptr);
    vkFreeMemory(device_app.device(), textureImageMemory_, nullptr);
    vkDestroySampler(device_app.device(), textureSampler_, nullptr);
    vkDestroyImageView(device_app.device(), textureImageView_, nullptr);

}



void JCubemap::createCubemapImage(const std::string& path, JDevice& device_app) {


    int width, height;
    const float* pixels = stbi_loadf(path.data(), &width, &height, nullptr, 4);
        if (!pixels) {
            std::string reason = stbi_failure_reason();
            throw std::runtime_error("failed to load cubemap image ' " + path + " ' : "+ reason);}
        if (width<=0||height<=0){
            throw std::runtime_error("Loaded cubemap image has invalid dimensions!");
        }

    std::cout << "DEBUG: HDR loaded successfully: " << width << "x" << height << std::endl;

    JBitmap in(width, height, 4, eJBitmapFormat_Float, pixels);
    std::cout << "DEBUG: JBitmap created successfully" << std::endl;
    
    //convert to vertical cross
    std::cout << "DEBUG: Starting convertEquirectangularMapToVerticalCross..." << std::endl;
    JBitmap out = convertEquirectangularMapToVerticalCross(in);
    std::cout << "DEBUG: convertEquirectangularMapToVerticalCross completed" << std::endl;
    
    stbi_image_free((void*)pixels);
    
    //write to disk to debug
    // stbi_write_hdr("./loaded_hdr_toVertical.hdr", out.w_, out.h_, out.channels_,
    //                 (const float*)out.data_.data());
    std::cout << "DEBUG: Starting convertVerticalCrossToCubeMapFaces..." << std::endl;
    cubemap_ = convertVerticalCrossToCubeMapFaces(out);  //create cubemap
    std::cout << "DEBUG: convertVerticalCrossToCubeMapFaces completed" << std::endl;

    VkDeviceSize faceSize = cubemap_.w_ * cubemap_.h_ * cubemap_.channels_ * JBitmap::getBytesPerChannel(cubemap_.format_) ; //float, 4 channels
    std::cout<< "DEBUG: cubemap size" << faceSize<< std::endl;
    VkDeviceSize totalSize = faceSize * 6;
    std::cout << "DEBUG: cubemap total size = " << totalSize << " faceSize=" << faceSize << std::endl;


    //find miplevels figure
    mipLevels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(cubemap_.w_, cubemap_.h_))))+1;
    //tex width is one cubemap_ face width
    texWidth = cubemap_.w_;
    texHeight = cubemap_.h_;
    texChannels = cubemap_.channels_;




    // Debug checks
    std::cout << "DEBUG: cubemap_.data_.size() = " << cubemap_.data_.size() << std::endl;
    std::cout << "DEBUG: cubemap_.depth_ = " << cubemap_.depth_ << std::endl;
    std::cout << "DEBUG: cubemap dimensions: " << cubemap_.w_ << "x" << cubemap_.h_ << "x" << cubemap_.channels_ << std::endl;

    // Verify data size matches expectation
    if (cubemap_.data_.size() != totalSize) {
        throw std::runtime_error("Cubemap data size mismatch!");
    }
    if (cubemap_.depth_ != 6) {
        throw std::runtime_error("Cubemap depth is not 6!");
    }

    //Staging buffer
    JBuffer stagingBuffer(device_app, totalSize, 
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* data;
    vkMapMemory(device_app.device(), stagingBuffer.bufferMemory(), 0, stagingBuffer.getSize(), 0, &data);
    memcpy(data, cubemap_.data_.data(), static_cast<size_t>(totalSize));
    vkUnmapMemory(device_app.device(), stagingBuffer.bufferMemory());

    // image create info
    auto imageInfo = ImageCreateInfoBuilder(texWidth, texHeight)
                    .mipLevels(mipLevels_)
                    .arrayLayers(cubemap_.depth_)
                    .format(cubemap_.getVkFormat())
                    .flags(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) // for cubemap_ especially
                    .usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_SAMPLED_BIT)
                    .getInfo();
    VkResult result = device_app.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage_, textureImageMemory_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create cubemap image! VkResult: " + std::to_string(result));
    }
    std::cout << "DEBUG: Cubemap image created successfully" << std::endl;
    
    JCommandBuffer commandBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    commandBuffer.beginSingleTimeCommands();

    // Transition all 6 cubemap faces manually since transitionImageLayout only handles layerCount=1
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = textureImage_;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels_;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 6;  // All 6 cubemap faces
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(commandBuffer.getCommandBuffer(),
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
        0, nullptr, 0, nullptr, 1, &barrier);

    // need able to copy 6 times
    // copyBufferToImage(commandBuffer.getCommandBuffer(), stagingBuffer.buffer(), textureImage_, 
    //     static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));


    std::array<VkBufferImageCopy, 6> regions{};
    for (uint32_t i = 0; i < 6; ++i) {
        regions[i].bufferOffset = faceSize * i;
        regions[i].bufferRowLength = 0;
        regions[i].bufferImageHeight = 0;
        regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        regions[i].imageSubresource.mipLevel = 0;
        regions[i].imageSubresource.baseArrayLayer = i; // 逐层
        regions[i].imageSubresource.layerCount = 1;
        regions[i].imageOffset = {0, 0, 0};
        regions[i].imageExtent = { (uint32_t)texWidth, (uint32_t)texHeight, 1 };
    }
    
    vkCmdCopyBufferToImage(
        commandBuffer.getCommandBuffer(),
        stagingBuffer.buffer(),
        textureImage_,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        static_cast<uint32_t>(regions.size()),
        regions.data()
    );

    // Transition to shader read-only layout
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer.getCommandBuffer(),
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr, 0, nullptr, 1, &barrier);
    
    commandBuffer.endSingleTimeCommands(device_app.graphicsQueue());

    //generate mipmaps
    // generateMipmaps(textureImage_, cubemap_.getVkFormat(), texWidth, texHeight, mipLevels_);

}




void JCubemap::createCubemapImageView(){
    auto viewInfo = ImageViewCreateInfoBuilder(textureImage_)
                    .mipLevels(0, mipLevels_)
                    .viewType(VK_IMAGE_VIEW_TYPE_CUBE)
                    .format(cubemap_.getVkFormat())
                    .arrayLayers(0, static_cast<uint32_t>(cubemap_.depth_))
                    .getInfo();
    VkResult result = device_app.createImageViewWithInfo(viewInfo, textureImageView_);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create cubemap image view! VkResult: " + std::to_string(result));
    }
    std::cout << "DEBUG: Cubemap image view created successfully" << std::endl;
}




void JCubemap::createCubemapSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device_app.physicalDevice(), &properties);

    auto samplerInfo = SamplerCreateInfoBuilder()
                        .addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                        .maxLod(mipLevels_)
                        .compareOp(VK_COMPARE_OP_NEVER)
                        .maxAnisotropy(properties.limits.maxSamplerAnisotropy)
                        .getInfo();

    if (vkCreateSampler(device_app.device(), &samplerInfo, nullptr, &textureSampler_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
    std::cout << "DEBUG: Cubemap sampler created successfully" << std::endl;
}




















