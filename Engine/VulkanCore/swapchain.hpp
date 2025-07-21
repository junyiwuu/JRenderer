#pragma once
#include <vulkan/vulkan.h>

#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>
#include <memory>
#include <cassert>
class JDevice;
class JWindow;


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
    VkFormat getSwapChainImageFormat() const {return swapChainImageFormat_;}

    VkImage getColorImage()                                 {return colorImage_;}
    VkImageView getColorImageView() const                   {return colorImageView_;}
    VkImage getDepthImage()                                 {return depthImage_;}
    VkImageView getDepthImageView() const                   {return depthImageView_;}
    std::vector<VkImageView> getSwapChainImageView() const  {return swapChainImageViews_;}
    std::vector<VkImage> getSwapChainImage()                {return swapChainImages_;}


    VkSemaphore getCurrentImageAvailableSemaphore(int index) {return imageAvailableSemaphores_[index];}
    VkSemaphore getCurrentRenderFinishedSemaphore(int index) {return renderFinishedSemaphores_[index];}
    VkFence& getCurrentInFlightFence(int index){ return inFlightFences_[index];}
    
    
    VkFormat findDepthFormat() const;



private:    
    JDevice& device_app;
    JWindow& window_app;

    VkSwapchainKHR swapChain_;
    std::shared_ptr<JSwapchain> oldSwapChain_;
    
    VkFormat swapChainImageFormat_;
    VkExtent2D swapChainExtent_;

    std::vector<VkImage> swapChainImages_;
    std::vector<VkImageView> swapChainImageViews_;
    std::vector<VkFramebuffer> swapChainFramebuffers_;

    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> renderFinishedSemaphores_;
    std::vector<VkFence> inFlightFences_;
    
    VkImage depthImage_;
    VkDeviceMemory depthImageMemory_;
    VkImageView depthImageView_;
    VkImage colorImage_;
    VkDeviceMemory colorImageMemory_;
    VkImageView colorImageView_;

    void init();
    void createSwapChain();
    void createImageViews();


    void cleanupSwapChain();

    void createSyncObjects();

    void createDepthResources() ;
    void createColorResources();
    
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) ;
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

};