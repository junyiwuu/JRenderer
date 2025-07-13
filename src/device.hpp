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

    VkDevice device() { return device_; }
    VkSurfaceKHR surface() { return surface_; }
    VkQueue graphicsQueue() { return graphicsQueue_; }
    VkQueue presentQueue() { return presentQueue_; }
    VkPhysicalDevice physicalDevice() {return physicalDevice_;}
    VkCommandPool getCommandPool() {return commandPool_;}


    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice_); }
    SwapChainSupportDetails getSwapChainSupport() {return querySwapChainSupport(physicalDevice_);}
    VkFormat findSupportFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    // memory allocation      


    VkImageView createImageViewWithInfo(const VkImageViewCreateInfo& imageViewInfo, VkImageView& imageView);

    void createImage(uint32_t width, uint32_t height, 
            uint32_t mipLevels ,VkFormat format, 
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
            VkImage& image, VkDeviceMemory& imageMemory);


    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void createImageWithInfo(const VkImageCreateInfo &imageInfo, 
        VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

            

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



    // will be checked if supported
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions={VK_KHR_SWAPCHAIN_EXTENSION_NAME};  // -> content is VK_KHR_swapchain

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
};



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

    VkImageViewCreateInfo getInfo() const {return viewInfo;}

};
































