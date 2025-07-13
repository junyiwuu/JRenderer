#pragma once
#include <vulkan/vulkan.h>

#include "device.hpp"
#include "buffer.hpp"
#include "utility.hpp"






class JTexture{

public:

    JTexture(const std::string& path, JDevice& device);
    ~JTexture();



private:

    JDevice& device_app;

    int texWidth;
    int texHeight;
    int texChannels;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkSampler textureSampler;

    void createTextureImage(const std::string& path, JDevice& device_app);    
    void transitionImageLayout(VkImage image, VkFormat format, 
        VkImageLayout oldLayout, VkImageLayout newLayout) ;

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) ;
    void createTextureSampler() ;
  










};





































