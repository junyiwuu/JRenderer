#pragma once
#include <vulkan/vulkan.hpp>



#include "../utility.hpp"
#include "bitmap.hpp"



class JDevice;




class JTexture{

public:
    
    JTexture(const std::string& path, JDevice& device);
    virtual ~JTexture();
    
    VkImageView textureImageView() const            {return textureImageView_;}
    VkSampler textureSampler() const                {return textureSampler_;}
    int getTextureWidth() const                     {return texWidth;}
    int getTextureHeight() const                    {return texHeight;}
    const VkDescriptorImageInfo& getDescriptorImageInfo() const {return descriptorImageInfo_;}

    static void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) ;
    
private:
    void createTextureImage(const std::string& path, JDevice& device_app);    
    void createTextureImageView();   
    void createTextureSampler() ;

protected:

    JDevice& device_app;

    int                         texWidth;
    int                         texHeight;
    int                         texChannels;
    uint32_t                    mipLevels_;
    VkImage                     textureImage_;
    VkImageView                 textureImageView_;
    VkDeviceMemory              textureImageMemory_;
    VkSampler                   textureSampler_;
    VkDescriptorImageInfo       descriptorImageInfo_;

 
    void createDescriptorInfo();
    void generateMipmaps(VkImage image, VkFormat imageFormat, 
        int32_t texWidth, int32_t texHeight, uint32_t mipLevels);


protected:
    // protected constructor for child class
    JTexture(JDevice& device)
      : device_app(device),
        texWidth(0), texHeight(0), texChannels(0),
        mipLevels_(0),
        textureImage_(VK_NULL_HANDLE),
        textureImageView_(VK_NULL_HANDLE),
        textureImageMemory_(VK_NULL_HANDLE),
        textureSampler_(VK_NULL_HANDLE)
    {}

};

//---------------------------------------------------------

class JCubemap: public JTexture{
public:
    JCubemap(const std::string& path, JDevice& device);
    ~JCubemap() override;

private:
    JBitmap cubemap_;

    void createCubemapImage(const std::string& path, JDevice& device);
    void createCubemapImageView();
    void createCubemapSampler();


};

























//---------------------------------------------------
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
    SamplerCreateInfoBuilder& maxLod(float _maxLod){
        samplerInfo.maxLod = _maxLod; return *this; } 
    SamplerCreateInfoBuilder& compareOp(VkCompareOp _compareOp){
        samplerInfo.maxLod = _compareOp; return *this; } 

    VkSamplerCreateInfo getInfo() const {return samplerInfo;}
};
































