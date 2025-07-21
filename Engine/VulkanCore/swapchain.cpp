#include "swapchain.hpp"
#include "window.hpp"
#include "device.hpp"

JSwapchain::JSwapchain(JDevice& device, JWindow& window):
        device_app(device), window_app(window)  {
    init();
}

JSwapchain::JSwapchain(JDevice& device, JWindow& window, std::shared_ptr<JSwapchain> previous):
        device_app(device), window_app(window), oldSwapChain_(previous)  {
    init();
    oldSwapChain_ = nullptr;

}


JSwapchain::~JSwapchain(){
    cleanupSwapChain();

    for (size_t i = 0; i < JSwapchain::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device_app.device(), renderFinishedSemaphores_[i], nullptr);
        vkDestroySemaphore(device_app.device(), imageAvailableSemaphores_[i], nullptr);
        vkDestroyFence(device_app.device(), inFlightFences_[i], nullptr);
    }
}


void JSwapchain::init() {
    createSwapChain();
    createImageViews();

    createColorResources();
    createDepthResources();

    createSyncObjects();
}

void JSwapchain::cleanupSwapChain() {
    vkDestroyImageView(device_app.device(), colorImageView_, nullptr);
    vkDestroyImage(device_app.device(), colorImage_, nullptr);
    vkFreeMemory(device_app.device(), colorImageMemory_, nullptr);
    vkDestroyImageView(device_app.device(), depthImageView_, nullptr);
    vkDestroyImage(device_app.device(), depthImage_, nullptr);
    vkFreeMemory(device_app.device(), depthImageMemory_, nullptr);

    for (auto framebuffer : swapChainFramebuffers_) {
        vkDestroyFramebuffer(device_app.device(), framebuffer, nullptr);
    }

    for (auto imageView : swapChainImageViews_) {
        vkDestroyImageView(device_app.device(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(device_app.device(), swapChain_, nullptr);
}


bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat JSwapchain::findDepthFormat() const {
    return device_app.findSupportedFormat(
    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}




void JSwapchain::createSwapChain() {
    SwapChainSupportDetails swapChainSupport =  device_app.getSwapChainSupport();

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = device_app.surface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices =  device_app.findPhysicalQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    // if oldswapchain=nullptr就创建vknullhandle，否则就用老的swapchain
    createInfo.oldSwapchain = oldSwapChain_ == nullptr ? VK_NULL_HANDLE : oldSwapChain_->swapChain_;

    if (vkCreateSwapchainKHR(device_app.device(), &createInfo, nullptr, &swapChain_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device_app.device(), swapChain_, &imageCount, nullptr);
    swapChainImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_app.device(), swapChain_, &imageCount, swapChainImages_.data());

    swapChainImageFormat_ = surfaceFormat.format;
    swapChainExtent_ = extent;
}



void JSwapchain::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();
    auto imageInfo = ImageCreateInfoBuilder(swapChainExtent_.width, swapChainExtent_.height)
                    .format(depthFormat)
                    .samples(device_app.msaaSamples())
                    .usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
                    .getInfo();
    VkResult res_createimg = device_app.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage_, depthImageMemory_);
    assert(res_createimg == VK_SUCCESS && "failed to create depth image in swapchain!");
    
    auto viewInfo = ImageViewCreateInfoBuilder(depthImage_)
                    .format(depthFormat)
                    .aspectMask(VK_IMAGE_ASPECT_DEPTH_BIT)
                    .getInfo();
    VkResult res_createview = device_app.createImageViewWithInfo(viewInfo,  depthImageView_);
    assert(res_createview == VK_SUCCESS && "failed to create depth image view in swapchain");
}


void JSwapchain::createImageViews() {
    swapChainImageViews_.resize(swapChainImages_.size());

    for (size_t i = 0; i < swapChainImages_.size(); ++i) {
        auto viewInfo = ImageViewCreateInfoBuilder(swapChainImages_[i])
                        .format(swapChainImageFormat_)
                        .componentsRGBA(VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY)
                        .getInfo();
        VkResult result =  device_app.createImageViewWithInfo(viewInfo, swapChainImageViews_[i]);
        if(result!= VK_SUCCESS){
            std::cerr << "swapchain image view creation failed at index : " << i ;
            assert(result == VK_SUCCESS && "swapchain image views create failed!");  }
        }
}

void JSwapchain::createColorResources() {
    VkFormat colorFormat = swapChainImageFormat_;
    auto imageInfo = ImageCreateInfoBuilder(swapChainExtent_.width, swapChainExtent_.height)
                    .samples(device_app.msaaSamples())
                    .format(colorFormat)
                    .usage(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                    .getInfo();
    VkResult res = device_app.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage_, colorImageMemory_);
    assert(res == VK_SUCCESS && "failed to create color image for color resources!");

    auto viewInfo = ImageViewCreateInfoBuilder(colorImage_)
                    .format(colorFormat)
                    .getInfo();
    VkResult res_view = device_app.createImageViewWithInfo(viewInfo, colorImageView_);
    assert(res_view == VK_SUCCESS && "failed to create color image view for color resources!");

}




VkSurfaceFormatKHR JSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}


VkPresentModeKHR JSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D JSwapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window_app.getGLFWwindow(), &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}


void JSwapchain::createSyncObjects() {
    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device_app.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device_app.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS ||
            vkCreateFence(device_app.device(), &fenceInfo, nullptr, &inFlightFences_[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}





























