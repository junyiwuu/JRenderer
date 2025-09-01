#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>


#include "../utility.hpp"
#include "bitmap.hpp"



class JDevice;
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//CONFIG area

struct TextureConfig{
    //common
    VkFormat                format;


    //image
    VkImageType             imageType;
    VkImageUsageFlags       usageFlags;
    VkImageCreateFlags      createFlags; //for cube compatible
    VkExtent3D              extent; //dimension - w, h, 1
    std::optional<uint32_t> mipLevels;

    VkImageLayout           newLayout;
    
    //imageview
    VkImageViewType         viewType;  //2D or cube
    uint32_t                arrayLayers;


    //sampler
    std::optional<VkSamplerAddressMode>    addressModeU;
    std::optional<VkSamplerAddressMode>    addressModeV;
    std::optional<VkSamplerAddressMode>    addressModeW;
    std::optional<VkCompareOp>             compareOp;

};

//when read general textures, it si from vk_layout_undefined -> vk_layout_transfer_dst -> vk_layout_shader_read_only_optimizal

inline TextureConfig CubeMapConfig(uint32_t width, uint32_t height, VkFormat format, uint32_t mipLevels){

    TextureConfig config;
    config.imageType        = VK_IMAGE_TYPE_2D;
    config.viewType         = VK_IMAGE_VIEW_TYPE_CUBE;
    
    config.extent           = {width, height,1 };
    config.format           = format;
    config.arrayLayers      = 6;
    
    config.createFlags      = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    config.usageFlags       = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    
    config.addressModeU     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    config.addressModeV     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    config.addressModeW     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    config.compareOp        = VK_COMPARE_OP_NEVER;

    config.newLayout        = VK_IMAGE_LAYOUT_GENERAL;

    return config;
    
}

inline TextureConfig PrefilterEnvMapConfig(uint32_t width, uint32_t height, VkFormat format  )
{
    
    uint32_t mips_  = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))))+1;
    TextureConfig config = CubeMapConfig(width, height, format, mips_);

    config.usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;

    return config;
}



///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


//create empty texture. should be able to inheritance
//NO DATA WRITE IN
class JTextureBase{
public:
    JTextureBase(JDevice& device, TextureConfig& textureConfig);
    virtual ~JTextureBase();

    //getter
    VkImage textureImage() const                    {return textureBaseImage_; }
    VkImageView textureImageView() const            {return textureBaseImageView_;}
    VkSampler textureSampler() const                {return textureBaseSampler_;}
    int getTextureWidth() const                     {return texWidth;}
    int getTextureHeight() const                    {return texHeight;}
    int getTextureChannels() const                  {return texChannels;}
    uint32_t getMipLevels() const                   {return mipLevels_;}

    const VkDescriptorImageInfo& getDescriptorImageInfo() const {return descriptorImageInfo_;}

    //functions
    VkImageView switchViewForMip(uint32_t selectMip, VkImageViewType vType);

private:


protected:
    JDevice& device_app;
    TextureConfig config_;



    void createTextureBase();

    int                         texWidth;
    int                         texHeight;
    int                         texChannels;
    uint32_t                    mipLevels_;
    VkImage                     textureBaseImage_;
    VkImageView                 textureBaseImageView_;
    VkDeviceMemory              textureBaseImageMemory_;
    VkSampler                   textureBaseSampler_;
    VkDescriptorImageInfo       descriptorImageInfo_;

};    


// class JPrefilterEnvMap: public JTextureBase{
// public:
//     JPrefilterEnvMap(const JCubemap& cubemapBase, JDevice& device);
//     ~JPrefilterEnvMap() override;
        

// private:


// };







///////////////////////////////////////////////////////////////////////////////////////////
//legacy one, create texture for asset
class JTexture{

public:
    
    JTexture(const std::string& path, JDevice& device);
    virtual ~JTexture();
    
    VkImageView textureImageView() const            {return textureImageView_;}
    VkSampler textureSampler() const                {return textureSampler_;}
    int getTextureWidth() const                     {return texWidth;}
    int getTextureHeight() const                    {return texHeight;}
    int getTextureChannels() const                  {return texChannels;}
    uint32_t getMipLevels() const                   {return mipLevels_;}
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
        int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t layerCount=1);


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
    void copyBufferToImage_multiple(VkCommandBuffer commandBuffer,
        VkBuffer buffer, VkImage image, 
        uint32_t imgWidth, uint32_t imgHeight, 
        VkDeviceSize bufferOffset, uint32_t layers);
};



class JCubemapAlter: public JTexture{
    public:
        JCubemapAlter(const JCubemap& cubemapBase, JDevice& device);
        ~JCubemapAlter() override;
            
    private:
        JBitmap cubemap_;

        void createTextureImage(const JCubemap& cubemapBase, JDevice& device_app);    
        void createTextureImageView();   
        void createTextureSampler() ;

        void copyBufferToImage_multiple(VkCommandBuffer commandBuffer,
            VkBuffer buffer, VkImage image, 
            uint32_t imgWidth, uint32_t imgHeight, 
            VkDeviceSize bufferOffset, uint32_t layers);
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
        samplerInfo.compareOp = _compareOp; return *this; } 

    VkSamplerCreateInfo getInfo() const {return samplerInfo;}
};
































