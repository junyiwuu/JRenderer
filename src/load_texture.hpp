#pragma once
#include <vulkan/vulkan.h>

#include "device.hpp"
#include "buffer.hpp"
#include "utility.hpp"






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
    VkImage textureImage_;
    VkImageView textureImageView_;
    VkDeviceMemory textureImageMemory_;
    VkSampler textureSampler_;

    void createTextureImage(const std::string& path, JDevice& device_app);    
    void createTextureImageView();
    void transitionImageLayout(VkImage image, VkFormat format, 
        VkImageLayout oldLayout, VkImageLayout newLayout) ;

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) ;
    void createTextureSampler() ;
  










};





































