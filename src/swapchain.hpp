#pragma once
#include <vulkan/vulkan.h>
#include "window.hpp"
#include "device.hpp"
#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>
#include <memory>
#include <cassert>



class JSwapchain{

public:
     
    static constexpr int MAX_FRAMES_IN_FLIGHT = 3;
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

    VkSemaphore getCurrentImageAvailableSemaphore(int index) {return imageAvailableSemaphores_[index];}
    VkSemaphore getCurrentRenderFinishedSemaphore(int index) {return renderFinishedSemaphores_[index];}
    VkFence& getCurrentInFlightFence(int index){ return inFlightFences_[index];}
    




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

    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> renderFinishedSemaphores_;
    std::vector<VkFence> inFlightFences_;
    
    VkImage depthImage_;
    VkDeviceMemory depthImageMemory_;
    VkImageView depthImageView_;


    void init();
    void createSwapChain();
    void createImageViews();
    void createFramebuffers();

    void cleanupSwapChain();
    void createRenderPass();
    void createSyncObjects();

    void createDepthResources() ;
    
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) ;
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkFormat findDepthFormat();
};