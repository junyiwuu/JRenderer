#pragma once
#include <vulkan/vulkan.h>
#include "window.hpp"
#include "utility.hpp"
#include <vector>
#include <iostream>
#include <cstring>
#include <optional>
#include <set>





struct QueueFamilyIndices{
    std::optional<uint32_t> graphicsFamily;  //optional is a wrapper that contains no value until you assign something to it
    std::optional<uint32_t> presentFamily;

    bool isComplete(){ return graphicsFamily.has_value()&&presentFamily.has_value();}
};

struct SwapChainSupportDetails{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

//////-----------------------------------------------------------------

class JDevice{

public:
    JDevice(JWindow& widnow);
    ~JDevice();

    JDevice(const JDevice&) = delete;
    JDevice &operator=(const JDevice&) = delete;

    VkDevice device() const { return device_; }
    VkSurfaceKHR surface() const { return surface_; }
    VkQueue graphicsQueue() const { return graphicsQueue_; }
    VkQueue presentQueue() const { return presentQueue_; }
    VkPhysicalDevice physicalDevice() const {return physicalDevice_;}
    VkCommandPool getCommandPool() const {return commandPool_;}
    VkPhysicalDeviceDriverProperties getDriverProperties() const {return driverProperties_;}

    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice_); }
    SwapChainSupportDetails getSwapChainSupport() {return querySwapChainSupport(physicalDevice_);}
    VkFormat findSupportFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    // memory allocation      


    VkResult createImageViewWithInfo(const VkImageViewCreateInfo& imageViewInfo, VkImageView& imageView);
    VkResult createImageWithInfo(const VkImageCreateInfo &imageInfo, 
            VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image,  
        VkImageLayout oldLayout, VkImageLayout newLayout,
        VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, 
        VkImageAspectFlags aspectMasek,  uint32_t mipLevels);

    VkSampleCountFlagBits msaaSamples() const {return msaaSamples_;}

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, 
        VkImageTiling tiling, VkFormatFeatureFlags features);

    
    
            

private:    
    JWindow& window_app;
    VkInstance instance_;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_; //logical device
    VkQueue graphicsQueue_;
    VkSurfaceKHR surface_;
    VkQueue presentQueue_;
    VkCommandPool commandPool_;
    VkSampleCountFlagBits msaaSamples_ = VK_SAMPLE_COUNT_1_BIT;
    VkPhysicalDeviceDriverProperties driverProperties_ = {};

    // will be checked if supported
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions={
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME, 
                    VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME,
                    };  // -> content is VK_KHR_swapchain

    #ifdef NDEBUG
        const bool enableValidationLayers=false;
    #else
        const bool enableValidationLayers=true;
    #endif

    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool(); 

    //helpler  ---  device
    bool isDeviceSuitable(VkPhysicalDevice device);
    int rateDeviceSuitability(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();

    void checkDriverProperties();


    //helpler  ---  validation
    bool checkValidationLayerSupport();


    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    //helpler  ---  debug
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, 
        const VkAllocationCallbacks* pAllocator);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
        const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) ;


    bool hasStencilComponent(VkFormat format);
 
    //mmsa
    VkSampleCountFlagBits getMaxUsableSampleCount();

};


////////////////////////////////////////////////////////////////////////////////////////////////////
struct ImageViewCreateInfoBuilder{
    VkImageViewCreateInfo viewInfo{};
    ImageViewCreateInfoBuilder(VkImage image){
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
    }

    ImageViewCreateInfoBuilder& viewType(VkImageViewType _type){
        viewInfo.viewType = _type; return *this; }

    ImageViewCreateInfoBuilder& format(VkFormat _format){
        viewInfo.format = _format; return *this; }

    ImageViewCreateInfoBuilder& aspectMask(VkImageAspectFlags _aspectMask){
        viewInfo.subresourceRange.aspectMask = _aspectMask; return *this; }
    
    ImageViewCreateInfoBuilder& mipLevels(uint32_t _baseMipLevel, uint32_t _levelCount){
        viewInfo.subresourceRange.baseMipLevel = _baseMipLevel;
        viewInfo.subresourceRange.levelCount = _levelCount;
        return *this; }

    ImageViewCreateInfoBuilder& arrayLayers(uint32_t _baseArrayLayer, uint32_t _layerCount){
    viewInfo.subresourceRange.baseArrayLayer = _baseArrayLayer;
    viewInfo.subresourceRange.layerCount     = _layerCount;
    return *this; }

    ImageViewCreateInfoBuilder& componentsRGBA(VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a ){
        viewInfo.components.r = r;
        viewInfo.components.g = g;
        viewInfo.components.b = b;
        viewInfo.components.a = a;
        return *this; }

    VkImageViewCreateInfo getInfo() const {return viewInfo;}

};


struct ImageCreateInfoBuilder{

    VkImageCreateInfo imageInfo{};
    ImageCreateInfoBuilder(uint32_t texWidth, uint32_t texHeight){
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = texWidth;
        imageInfo.extent.height = texHeight;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;  //for multisampling
        imageInfo.flags = 0;
    }

    ImageCreateInfoBuilder& imageCreateFlags(VkImageCreateFlags _flags){
        imageInfo.flags = _flags; return *this; }
    ImageCreateInfoBuilder& imageType(VkImageType _type){
        imageInfo.imageType = _type; return *this; }
    ImageCreateInfoBuilder& extentDepth(uint32_t _depth){
        imageInfo.extent.depth = _depth; return *this; }
    ImageCreateInfoBuilder& mipLevels(uint32_t _mipLevels){
        imageInfo.mipLevels = _mipLevels; return *this; }
    ImageCreateInfoBuilder& arrayLayers(uint32_t _arrayLayers){
        imageInfo.arrayLayers = _arrayLayers; return *this; }

    ImageCreateInfoBuilder& tiling(VkImageTiling _tiling){
        imageInfo.tiling = _tiling; return *this; }
    ImageCreateInfoBuilder& usage(VkImageUsageFlags _usage){
        imageInfo.usage = _usage; return *this; }
    ImageCreateInfoBuilder& sharingMode(VkSharingMode _sharingMode){
        imageInfo.sharingMode = _sharingMode; return *this; }
    ImageCreateInfoBuilder& samples(VkSampleCountFlagBits _samples){
            imageInfo.samples = _samples; return *this; }
    ImageCreateInfoBuilder& initialLayout(VkImageLayout _initialLayout){
        imageInfo.initialLayout = _initialLayout; return *this; }
    ImageCreateInfoBuilder& format(VkFormat _format){
        imageInfo.format = _format; return *this; }
        
    VkImageCreateInfo getInfo() const {return imageInfo;}



};































