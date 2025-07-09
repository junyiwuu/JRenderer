#pragma once
#include <vulkan/vulkan.h>
#include "window.hpp"
#include "device.hpp"
#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>
#include <memory>




class JSwapchain{

public:
     
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    JSwapchain(JDevice& device, JWindow& window);
    JSwapchain(JDevice& device, JWindow& window, std::shared_ptr<JSwapchain> previous);
    ~JSwapchain();

    JSwapchain(const JSwapchain&) = delete;
    JSwapchain &operator=(const JSwapchain&) = delete;

    VkSwapchainKHR swapChain() {return swapChain_;}
    VkExtent2D getSwapChainExtent() {return swapChainExtent_;}
    VkFramebuffer getFrameBuffer(int index) {return swapChainFramebuffers_[index];}
    VkFormat getSwapChainImageFormat() {return swapChainImageFormat_;}
    VkRenderPass renderPass() {return renderPass_;}

    VkResult acquireNextImage(uint32_t* imageIndex);
    




private:    
    JDevice& device_app;
    JWindow& window_app;

    
    VkSwapchainKHR swapChain_;
    std::shared_ptr<JSwapchain> oldSwapChain_;

    std::vector<VkImage> swapChainImages_;
    VkFormat swapChainImageFormat_;
    VkExtent2D swapChainExtent_;
    std::vector<VkImageView> swapChainImageViews_;
    std::vector<VkFramebuffer> swapChainFramebuffers_;
    VkRenderPass renderPass_;

    void init();
    void createSwapChain();
    void createImageViews();
    void createFramebuffers();

    void cleanupSwapChain();
    void createRenderPass();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) ;
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

};