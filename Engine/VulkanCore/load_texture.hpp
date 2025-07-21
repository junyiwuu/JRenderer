#pragma once
#include <vulkan/vulkan.hpp>



#include "utility.hpp"


class JDevice;




class JTexture{

public:

    JTexture(const std::string& path, JDevice& device);
    ~JTexture();
    
    VkImageView textureImageView() const {return textureImageView_;}
    VkSampler textureSampler() const {return textureSampler_;}


    static void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) ;


private:

    JDevice& device_app;

    int texWidth;
    int texHeight;
    int texChannels;
    uint32_t mipLevels_;
    VkImage textureImage_;
    VkImageView textureImageView_;
    VkDeviceMemory textureImageMemory_;
    VkSampler textureSampler_;

    void createTextureImage(const std::string& path, JDevice& device_app);    
    void createTextureImageView();

    
    void createTextureSampler() ;
  

    void generateMipmaps(VkImage image, VkFormat imageFormat, 
        int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

};




struct SamplerCreateInfoBuilder{
    VkSamplerCreateInfo samplerInfo{};
    SamplerCreateInfoBuilder(){

    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
    }
    SamplerCreateInfoBuilder& addressMode(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w){
        samplerInfo.addressModeU = u;
        samplerInfo.addressModeV = v;
        samplerInfo.addressModeW = w;
        return *this; }
    SamplerCreateInfoBuilder& maxAnisotropy(float _maxAni){
        samplerInfo.maxAnisotropy = _maxAni; return *this; }
    SamplerCreateInfoBuilder& borderColor(VkBorderColor _color){
        samplerInfo.borderColor = _color; return *this; }
    SamplerCreateInfoBuilder& mipmapMode(VkSamplerMipmapMode _mipmapMode){
        samplerInfo.mipmapMode = _mipmapMode; return *this; } 

    VkSamplerCreateInfo getInfo() const {return samplerInfo;}
};
































