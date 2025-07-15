#pragma once
#include <vulkan/vulkan.h>

#include "device.hpp"
#include "buffer.hpp"
#include "utility.hpp"
#include "commandBuffer.hpp"






class JTexture{

public:

    JTexture(const std::string& path, JDevice& device);
    ~JTexture();
    
    VkImageView textureImageView() const {return textureImageView_;}
    VkSampler textureSampler() const {return textureSampler_;}





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

    void copyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) ;
    void createTextureSampler() ;
  

    void generateMipmaps(VkImage image, VkFormat imageFormat, 
        int32_t texWidth, int32_t texHeight, uint32_t mipLevels);








};





































