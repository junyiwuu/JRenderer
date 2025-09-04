#include "load_texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include "cubemapUtils.hpp"
#include "../buffer.hpp"
#include "../device.hpp"
#include "../commandBuffer.hpp"

///////////////////////////////////////////////////////////////////////////////////////////
SamplerManager::SamplerManager(JDevice& device):
    device_app(device)
{
    //find maximum sampler anistropy
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device_app.physicalDevice(), &properties);
    maxSamplerAnisotropy_ = properties.limits.maxSamplerAnisotropy;
    initDefault();
}

void SamplerManager::initDefault(){
    //TextureGlobal
    VkSampler sampler_TextureGlobal = build(SamplerType::TextureGlobal);
    defaultSamplers_[SamplerType::TextureGlobal] = sampler_TextureGlobal;

    //skybox linear clamp
    VkSampler sampler_SkyboxLinearClamp = build(SamplerType::SkyboxLinearClamp);
    defaultSamplers_[SamplerType::SkyboxLinearClamp] = sampler_SkyboxLinearClamp;
}


void SamplerManager::destroy(){
    //destroy default samplers
    for(auto& pair:defaultSamplers_){
        vkDestroySampler(device_app.device(), pair.second, nullptr);   }
    defaultSamplers_.clear();
    //destory customized samplers. dont need to check if it has something already
    for(auto& pair: customSamplers_){
        vkDestroySampler(device_app.device(), pair.second, nullptr);   }
    customSamplers_.clear();
}

void SamplerManager::addSampler(const std::string& name, VkSampler newSampler){
    customSamplers_[name] = newSampler;
}


VkSampler SamplerManager::build(SamplerType samplerType){
    switch(samplerType){
        case SamplerType::TextureGlobal :
            {auto samplerInfo = SamplerCreateInfoBuilder()
                                .maxAnisotropy(maxSamplerAnisotropy_)
                                .getInfo();
            VkSampler sampler_;
            if (vkCreateSampler(device_app.device(), &samplerInfo, nullptr, &sampler_) != VK_SUCCESS) {
                throw std::runtime_error("failed to create TextureGlobal sampler!");         }
            return sampler_;
            break;}

        
        case SamplerType::SkyboxLinearClamp :
            {auto samplerInfo = SamplerCreateInfoBuilder()
                                .addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                                .compareOp(VK_COMPARE_OP_NEVER)
                                .maxAnisotropy(maxSamplerAnisotropy_)
                                .getInfo();
            VkSampler sampler_;
            if (vkCreateSampler(device_app.device(), &samplerInfo, nullptr, &sampler_) != VK_SUCCESS) {
                throw std::runtime_error("failed to create Skybox_LinearClamp sampler!");         }
            return sampler_;
            break;}
        
        default:
            throw std::runtime_error("Unknown SamplerType in SamplerManager::build()");
    }
}


VkSampler SamplerManager::getSampler(const std::string& name){
    auto pair = customSamplers_.find(name);
    if(pair!=customSamplers_.end()){
        return pair->second;    }
    throw std::runtime_error("customized sampler:" + name + "not found!");
}


VkSampler SamplerManager::getSampler(SamplerType samplerType){
    auto pair = defaultSamplers_.find(samplerType);
    if(pair!=defaultSamplers_.end()){
        return pair->second;  }
    throw std::runtime_error("default sampler:" + std::to_string(static_cast<int>(samplerType)) + "not found!");
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//need to add one automatically read channels function 

//控制所有的资源安排，创建销毁
//need to create texChannels based on the vkformat
JTextureBase::JTextureBase(JDevice& device, TextureConfig& textureConfig):
    device_app(device), config_(textureConfig)

{
    texWidth = config_.extent.width;
    texHeight = config_.extent.height;
    texChannels = bytesPerPixel(config_.format);


    //if not setting miplevel, means need to calculate
    if(config_.mipLevels){
        mipLevels_ = *config_.mipLevels;
    }else{
        mipLevels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight))))+1;
    }

    createTextureBase();

}


JTextureBase:: ~JTextureBase(){
    if(customSampler_){
        vkDestroySampler(device_app.device(), *customSampler_, nullptr);
    }
    vkDestroyImageView(device_app.device(), textureBaseImageView_, nullptr);
    vkDestroyImage(device_app.device(), textureBaseImage_, nullptr);
    vkFreeMemory(device_app.device(), textureBaseImageMemory_, nullptr);
}

void JTextureBase::createCustomSampler(const VkSamplerCreateInfo& samplerInfo){
    VkSampler tempSampler = VK_NULL_HANDLE;
    if (vkCreateSampler(device_app.device(), &samplerInfo, nullptr, &tempSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create customized sampler!"); }
        customSampler_ = tempSampler;
}



VkImageView JTextureBase::switchViewForMip(uint32_t selectMip, VkImageViewType vType){
    auto viewInfo = ImageViewCreateInfoBuilder(textureBaseImage_)
                    .viewType(vType)
                    .format(config_.format)
                    .mipLevels(selectMip, 1)
                    .arrayLayers(0, config_.arrayLayers)
                    .getInfo();

    VkImageView currentView;
    device_app.createImageViewWithInfo(viewInfo, currentView);
    return currentView;
}


void JTextureBase::createTextureBase(){
    auto imageInfo = ImageCreateInfoBuilder(texWidth, texHeight)
                    .imageType(config_.imageType)
                    .format(config_.format)
                    .arrayLayers(config_.arrayLayers)
                    .mipLevels(mipLevels_)
                    .flags(config_.createFlags)
                    .usage(config_.usageFlags)
                    .getInfo();
    if(device_app.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureBaseImage_, textureBaseImageMemory_)!=VK_SUCCESS){
        throw std::runtime_error("Failed to create VkImage for Texture Base");
    };

    //create image view
    auto viewInfo = ImageViewCreateInfoBuilder(textureBaseImage_)
                    .viewType(config_.viewType)
                    .format(config_.format)
                    .mipLevels(0, mipLevels_)
                    .arrayLayers(0, config_.arrayLayers)
                    .getInfo();
    if(device_app.createImageViewWithInfo(viewInfo, textureBaseImageView_)!=VK_SUCCESS){
        throw std::runtime_error("Failed to create VkImageView for Texture Base");
    };

    //transition image layout for all
    JCommandBuffer commandBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    commandBuffer.beginSingleTimeCommands();

    device_app.transitionImageLayout(commandBuffer.getCommandBuffer(), textureBaseImage_,
        VK_IMAGE_LAYOUT_UNDEFINED, config_.newLayout, 
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT, mipLevels_, config_.arrayLayers);

    commandBuffer.endSingleTimeCommands(device_app.graphicsQueue());

    if(config_.data){ //if user provide data
        VkDeviceSize imageSize = texWidth * texHeight * bytesPerPixel(config_.format) * config_.arrayLayers;
        JBuffer stagingBuffer(device_app, imageSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        void* dstData;
        vkMapMemory(device_app.device(), stagingBuffer.bufferMemory(), 0, stagingBuffer.getSize(), 0, &dstData);
        memcpy(dstData, *config_.data, static_cast<size_t>(imageSize));
        vkUnmapMemory(device_app.device(), stagingBuffer.bufferMemory());
        
        commandBuffer.beginSingleTimeCommands();
        copyBufferToImage(commandBuffer.getCommandBuffer(), stagingBuffer.buffer(), textureBaseImage_, 
            static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

        commandBuffer.endSingleTimeCommands(device_app.graphicsQueue());
        generateMipmaps(textureBaseImage_, config_.format , texWidth, texHeight, mipLevels_);   
    }

}

    

void JTextureBase::copyBufferToImage(VkCommandBuffer commandBuffer,VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
    VkDeviceSize bufferOffset, uint32_t layers) 
{
    //include the single layer or multi layer cases
    std::vector<VkBufferImageCopy> regions(layers);
    for(uint32_t i = 0 ; i<layers; ++i){
        regions[i].bufferOffset = bufferOffset * i;
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
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        static_cast<uint32_t>(regions.size()),
        regions.data()
    );
    // util::endSingleTimeCommands(device_app.device(), commandBuffer, device_app.getCommandPool(), device_app.graphicsQueue());
}



 void JTextureBase::generateMipmaps(VkImage image, VkFormat imageFormat, 
                      int32_t texWidth, int32_t texHeight, 
                      uint32_t mipLevels, uint32_t layerCount){

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(device_app.physicalDevice(), imageFormat, &formatProperties);

    if(!(formatProperties.optimalTilingFeatures&VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)){
        throw std::runtime_error("texture image format does not support linear blitting!"); }
    
    VkCommandBuffer commandBuffer = util::beginSingleTimeCommands(device_app.device(), device_app.getCommandPool());
    
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;
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
        blit.srcSubresource.layerCount = layerCount;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = layerCount;

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


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


//create 2dTexture automatically from path
JTexture2D::JTexture2D(JDevice& device, const std::string& path, VkFormat format):
     JTextureBase(device)
{
    config_ = createConfig(path, format);
    createTextureImage();
    createTextureImageView();
}

TextureConfig JTexture2D::createConfig(const std::string& path, VkFormat format){
    //here can read data directly
    pixels_ = stbi_load(path.data(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    
    mipLevels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight))))+1;
    if (!pixels_) {
        throw std::runtime_error("failed to load texture image!");}

    TextureConfig config;
    config.imageType        = VK_IMAGE_TYPE_2D;
    config.viewType         = VK_IMAGE_VIEW_TYPE_2D;
    config.extent           = {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),1 };
    config.format           = format;
    config.arrayLayers      = 1;
    config.usageFlags       = VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_SAMPLED_BIT;
    config.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    return config;
}


void JTexture2D::createTextureImage(){

    VkDeviceSize imageSize = texWidth * texHeight * 4; // Always 4 channels due to STBI_rgb_alpha; 
    //if i fill in texchannels not correct, fill in bytesperpixel(by vkformat) still not correct, need to investigate later
    JBuffer stagingBuffer(device_app, imageSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


    void* data;
    vkMapMemory(device_app.device(), stagingBuffer.bufferMemory(), 0, stagingBuffer.getSize(), 0, &data);
    memcpy(data, pixels_, static_cast<size_t>(imageSize));
    vkUnmapMemory(device_app.device(), stagingBuffer.bufferMemory());
    stbi_image_free(pixels_);

    auto imageInfo = ImageCreateInfoBuilder(texWidth, texHeight)
                .mipLevels(mipLevels_)
                .format(config_.format)
                .usage(config_.usageFlags )
                .getInfo();
    if(device_app.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureBaseImage_, textureBaseImageMemory_)!=VK_SUCCESS){
        throw std::runtime_error("Failed to create VkImage for Texture2D");
    };

    JCommandBuffer commandBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    commandBuffer.beginSingleTimeCommands();

    device_app.transitionImageLayout(commandBuffer.getCommandBuffer() ,textureBaseImage_,  
    VK_IMAGE_LAYOUT_UNDEFINED, config_.newLayout   ,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,   VK_PIPELINE_STAGE_TRANSFER_BIT,
    VK_IMAGE_ASPECT_COLOR_BIT, mipLevels_, 1);

    copyBufferToImage(commandBuffer.getCommandBuffer(), stagingBuffer.buffer(), textureBaseImage_, 
        static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    commandBuffer.endSingleTimeCommands(device_app.graphicsQueue());

    generateMipmaps(textureBaseImage_, config_.format , texWidth, texHeight, mipLevels_);   
}



void JTexture2D::createTextureImageView(){
    auto viewInfo = ImageViewCreateInfoBuilder(textureBaseImage_)
                    .viewType(VK_IMAGE_VIEW_TYPE_2D)
                    .format(config_.format)
                    .mipLevels(0, mipLevels_)
                    .getInfo();
    if(device_app.createImageViewWithInfo(viewInfo, textureBaseImageView_)!=VK_SUCCESS){
        throw std::runtime_error("Failed to create VkImageView for Texture2D");
    };
}


JTexture2D::~JTexture2D(){
}
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////



//create 2dTexture automatically from path
JSolidColor::JSolidColor(JDevice& device, float r, float g, float b):
     JTextureBase(device)
{
    texWidth = 4;
    texHeight = 4;
    texChannels = 4;
    config_ = createConfig();

    generateData(r, g, b);
    createTextureImage();
    createTextureImageView();
}

void JSolidColor::generateData(float r, float g, float b){
    //must be four channels, otherwise my gpu doesnt support three channels linear blitting
    uint8_t red   = static_cast<uint8_t>(r* 255.0f);
    uint8_t green = static_cast<uint8_t>(g* 255.0f);
    uint8_t blue  = static_cast<uint8_t>(b* 255.0f);

    int totalPixels = texWidth*texHeight;
    int totalBytes = totalPixels * texChannels;

    pixels_ = new uint8_t[totalBytes];

    for(int i = 0; i < totalPixels; ++i){
        pixels_[i * texChannels + 0] = red;
        pixels_[i * texChannels + 1] = green;
        pixels_[i * texChannels + 2] = blue;      
        pixels_[i * texChannels + 3] = 255.0f;   }
}


TextureConfig JSolidColor::createConfig(){  
    mipLevels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight))))+1;

    TextureConfig config;
    config.imageType        = VK_IMAGE_TYPE_2D;
    config.viewType         = VK_IMAGE_VIEW_TYPE_2D;
    config.extent           = {static_cast<uint32_t>(4), static_cast<uint32_t>(4),1 };
    config.format           = VK_FORMAT_R8G8B8A8_UNORM;
    config.arrayLayers      = 1;
    config.usageFlags       = VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT|VK_IMAGE_USAGE_SAMPLED_BIT;
    config.newLayout        = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    return config;
}


void JSolidColor::createTextureImage(){

    VkDeviceSize imageSize = texWidth * texHeight * texChannels; // 4 channels, integer, so each channel 1 byte
    JBuffer stagingBuffer(device_app, imageSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* data;
    vkMapMemory(device_app.device(), stagingBuffer.bufferMemory(), 0, stagingBuffer.getSize(), 0, &data);
    memcpy(data, pixels_, static_cast<size_t>(imageSize));
    vkUnmapMemory(device_app.device(), stagingBuffer.bufferMemory());

    auto imageInfo = ImageCreateInfoBuilder(texWidth, texHeight)
                .mipLevels(mipLevels_)
                .format(config_.format)
                .usage(config_.usageFlags )
                .getInfo();
    if(device_app.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureBaseImage_, textureBaseImageMemory_)!=VK_SUCCESS){
        throw std::runtime_error("Failed to create VkImage for Solid color");
    };

    JCommandBuffer commandBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    commandBuffer.beginSingleTimeCommands();

    device_app.transitionImageLayout(commandBuffer.getCommandBuffer() ,textureBaseImage_,  
        VK_IMAGE_LAYOUT_UNDEFINED, config_.newLayout   ,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,   VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT, mipLevels_, 1);

    copyBufferToImage(commandBuffer.getCommandBuffer(), stagingBuffer.buffer(), textureBaseImage_, 
        static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    commandBuffer.endSingleTimeCommands(device_app.graphicsQueue());

    generateMipmaps(textureBaseImage_, config_.format , texWidth, texHeight, mipLevels_);   
}



void JSolidColor::createTextureImageView(){
    auto viewInfo = ImageViewCreateInfoBuilder(textureBaseImage_)
                    .viewType(VK_IMAGE_VIEW_TYPE_2D)
                    .format(config_.format)
                    .mipLevels(0, mipLevels_)
                    .getInfo();
    if(device_app.createImageViewWithInfo(viewInfo, textureBaseImageView_)!=VK_SUCCESS){
        throw std::runtime_error("Failed to create VkImageView for Solid color");
    };
}


JSolidColor::~JSolidColor(){
    delete[] pixels_;
}





///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

JTexture::JTexture(const std::string& path, JDevice& device):
    device_app(device)    
{
    createTextureImage(path, device_app);
    createTextureImageView();
    createTextureSampler();
    createDescriptorInfo();
}


JTexture::~JTexture(){  //order is important
    vkDestroySampler(device_app.device(), textureSampler_, nullptr);
    vkDestroyImageView(device_app.device(), textureImageView_, nullptr);
    vkDestroyImage(device_app.device(), textureImage_, nullptr);
    vkFreeMemory(device_app.device(), textureImageMemory_, nullptr);

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
    if(device_app.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage_, textureImageMemory_)!=VK_SUCCESS){
        throw std::runtime_error("Failed to create VkImage for texture(old one)");
    };

    JCommandBuffer commandBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    commandBuffer.beginSingleTimeCommands();

    device_app.transitionImageLayout(commandBuffer.getCommandBuffer() ,textureImage_,  
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,   VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT, mipLevels_, 1);

    copyBufferToImage(commandBuffer.getCommandBuffer(), stagingBuffer.buffer(), textureImage_, 
        static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    commandBuffer.endSingleTimeCommands(device_app.graphicsQueue());

    generateMipmaps(textureImage_, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels_);

}


void JTexture::createTextureImageView(){
    auto viewInfo = ImageViewCreateInfoBuilder(textureImage_)
                    .viewType(VK_IMAGE_VIEW_TYPE_2D)
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
                      int32_t texWidth, int32_t texHeight, 
                      uint32_t mipLevels, uint32_t layerCount){

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
    barrier.subresourceRange.layerCount = layerCount;
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
        blit.srcSubresource.layerCount = layerCount;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = layerCount;

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




///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

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
    // vkDestroyImage(device_app.device(), textureImage_, nullptr);
    // vkFreeMemory(device_app.device(), textureImageMemory_, nullptr);
    // vkDestroySampler(device_app.device(), textureSampler_, nullptr);
    // vkDestroyImageView(device_app.device(), textureImageView_, nullptr);

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

    
    JBitmap in(width, height, 4, eJBitmapFormat_Float, pixels); 
    //convert to vertical cross
    JBitmap out = convertEquirectangularMapToVerticalCross(in);    
    stbi_image_free((void*)pixels);
    
    //write to disk to debug
    // stbi_write_hdr("./loaded_hdr_toVertical.hdr", out.w_, out.h_, out.channels_,
    //                 (const float*)out.data_.data());
    
    cubemap_ = convertVerticalCrossToCubeMapFaces(out);  //create cubemap
    VkDeviceSize faceSize = cubemap_.w_ * cubemap_.h_ * cubemap_.channels_ * JBitmap::getBytesPerChannel(cubemap_.format_) ; //float, 4 channels
    VkDeviceSize totalSize = faceSize * 6;

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
    
    JCommandBuffer commandBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    commandBuffer.beginSingleTimeCommands();

    device_app.transitionImageLayout(commandBuffer.getCommandBuffer(), textureImage_,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT, mipLevels_, 6);

    
    copyBufferToImage_multiple(commandBuffer.getCommandBuffer(), 
        stagingBuffer.buffer(), textureImage_,
        (uint32_t)texWidth, (uint32_t)texHeight,
        faceSize, 6   );

// no transition again, egerate mipmaps will transfer everything back

   
    commandBuffer.endSingleTimeCommands(device_app.graphicsQueue());

    //generate mipmaps for cubemap (6 layers)
    generateMipmaps(textureImage_, cubemap_.getVkFormat(), texWidth, texHeight, mipLevels_, 6);

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
        throw std::runtime_error("failed to create cubemap sampler!");
    }
}


void JCubemap::copyBufferToImage_multiple(VkCommandBuffer commandBuffer,
    VkBuffer buffer, VkImage image, 
    uint32_t imgWidth, uint32_t imgHeight, 
    VkDeviceSize bufferOffset, uint32_t layers) {

        std::vector<VkBufferImageCopy> regions(layers);
        for(uint32_t i = 0 ; i<layers; ++i){
            regions[i].bufferOffset = bufferOffset * i;
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
            commandBuffer,
            buffer,
            textureImage_,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            static_cast<uint32_t>(regions.size()),
            regions.data()
        );
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////








