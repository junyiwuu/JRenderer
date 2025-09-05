#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include "../utility.hpp"
#include "bitmap.hpp"
class JDevice;




inline int bytesPerPixel(VkFormat format){
    switch(format){
        case VK_FORMAT_R8_UNORM: 
        case VK_FORMAT_R8_SRGB: 
            return 1;

        case VK_FORMAT_R8G8_UNORM: 
        case VK_FORMAT_R8G8_SRGB:
            return 2;

        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SRGB:
            return 3;

        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
            return 4;

        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return 8;

        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return 16; // 32 bits × 4 channels

        default:
            throw std::runtime_error("bytesPerPixel() not implemented for this VkFormat");
    
    }
}



///sampler
//create global sampler

enum class SamplerType {
    TextureGlobal,

    SkyboxLinearClamp, //see in the viewport


};

//universe sampler, the miplevel is controlled by image view, not the sampler side

class SamplerManager{
public:
    explicit SamplerManager(JDevice& device);

    //including both customized sampler(later added) and default samplers

    //init all universal ones
    void initDefault();

    void addSampler(const std::string& name, VkSampler newSampler);

    VkSampler getSampler(const std::string& name); //get customized
    VkSampler getSampler(SamplerType samplerType); //get default

    void destroy();  //need explicit call
private:
    JDevice& device_app;
    float maxSamplerAnisotropy_;

    std::unordered_map<std::string, VkSampler> customSamplers_;
    std::unordered_map<SamplerType, VkSampler> defaultSamplers_;
    //individual build
    VkSampler build(SamplerType samplerType);
};

//individual one just use the samplerInfoBuilder.. all been added to samplers_, in the end they all be destroyed


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//CONFIG area

struct TextureConfig{
    //common
    VkFormat                format;
    uint32_t                channels;


    //image
    VkImageType             imageType{VK_IMAGE_TYPE_2D};
    VkImageUsageFlags       usageFlags;
    VkImageCreateFlags      createFlags{0}; //for cube compatible
    VkExtent3D              extent; //dimension - w, h, 1. this is for 3D texture
    std::optional<uint32_t> mipLevels;

    std::optional<void*>    data; //can provide, or just empty one

    VkImageLayout           newLayout{VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL};
    
    //imageview
    VkImageViewType         viewType{VK_IMAGE_VIEW_TYPE_2D};  //2D or cube
    uint32_t                arrayLayers{1};
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
    int getTextureWidth() const                     {return texWidth;}
    int getTextureHeight() const                    {return texHeight;}
    int getTextureChannels() const                  {return texChannels;}
    uint32_t getMipLevels() const                   {return mipLevels_;}

    //optional functions
    VkImageView switchViewForMip(uint32_t selectMip, VkImageViewType vType);
    //only use when has customized sampler.. doing this for manage customized sampler inside this texture life cycle
    void createCustomSampler(const VkSamplerCreateInfo& samplerInfo); //create sampler inside, so store in the class

    VkDescriptorImageInfo getDesImageInfo() const 
        {   if(!customSampler_.has_value()){throw std::runtime_error("No sampler assigned when creating this texture object");}
            return  VkDescriptorImageInfo{
                *customSampler_,
                textureBaseImageView_,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,   };        
        }
private:
    void createTextureBase();

protected:
    JDevice& device_app;
    TextureConfig config_;    

    int                         texWidth;
    int                         texHeight;
    int                         texChannels;
    uint32_t                    mipLevels_;
    VkImage                     textureBaseImage_;
    VkImageView                 textureBaseImageView_;
    VkDeviceMemory              textureBaseImageMemory_;

    std::optional<VkSampler>    customSampler_; //for unique customized sampler store inside

    //for use
    void copyBufferToImage(VkCommandBuffer commandBuffer,VkBuffer buffer, 
        VkImage image, uint32_t width, uint32_t height,
        VkDeviceSize bufferOffset = 0, uint32_t layers = 1) ;

    void generateMipmaps(VkImage image, VkFormat imageFormat, 
        int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t layerCount=1);

protected:
    // protected constructor for child class
    JTextureBase(JDevice& device)
      : device_app(device),
        texWidth(0), texHeight(0), texChannels(0),
        mipLevels_(0),
        textureBaseImage_(VK_NULL_HANDLE),
        textureBaseImageView_(VK_NULL_HANDLE),
        textureBaseImageMemory_(VK_NULL_HANDLE)
    {}
};    


class JTexture2D: public JTextureBase{

public:
    JTexture2D(JDevice& device, const std::string& path, VkFormat format);
    ~JTexture2D() override;

    VkDescriptorImageInfo getDescriptorImageInfo(VkSampler sampler) const 
        {return  VkDescriptorImageInfo{
            sampler,
            textureBaseImageView_,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,   };        
        }

private:
    stbi_uc* pixels_;
    TextureConfig createConfig(const std::string& path, VkFormat format);

    void createTextureImage();
    void createTextureImageView();
};

//---------------------------------------------------------------------------------------
class JSolidColor: public JTextureBase{

    public:
        JSolidColor(JDevice& device, float r, float g, float b);
        ~JSolidColor() override;
    
        VkDescriptorImageInfo getDescriptorImageInfo(VkSampler sampler) const 
            {return  VkDescriptorImageInfo{
                sampler,
                textureBaseImageView_,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,   };        
            }
    
    private:
        uint8_t* pixels_;
        TextureConfig createConfig();

        void generateData(float r, float g, float b);
    
        void createTextureImage();
        void createTextureImageView();
    };


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
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;  //when compareEnable is false, compareOP doesnt matter
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







namespace TexUtils{

void UploadKtxToTexture(JDevice& device,
        ktxTexture2* ktxTex,
        JTextureBase& dstTex,
        bool isCube);
    
}
    
























