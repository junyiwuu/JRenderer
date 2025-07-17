
#pragma once
#include <vulkan/vulkan.hpp>




class JSwapchain;
class JDevice;



class RenderTarget{

public:



    RenderTarget(JDevice& device, JSwapchain& swapChain);
    // ~RenderTarget();







private:
    JDevice& device_app;
    JSwapchain& swapchain_app;

    VkImage                     depthImage_;
    VkDeviceMemory              depthImageMemory_;
    VkImageView                 depthImageView_;
    vk::RenderingAttachmentInfo depthAttachmentInfo_;


    std::vector<VkImage>        colorImage_;
    VkDeviceMemory              colorImageMemory_;
    VkImageView                 colorImageView_;
    vk::RenderingAttachmentInfo colorAttachmentInfo_;

    std::vector<VkImageView> swapChainImageViews_;


    void createSwapChainImageViews();
    void createAttachments();



};

































