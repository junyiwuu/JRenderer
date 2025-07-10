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


    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    void createImage(uint32_t width, uint32_t height, 
            uint32_t mipLevels ,VkFormat format, 
            VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
            VkImage& image, VkDeviceMemory& imageMemory);


    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);


            

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

































