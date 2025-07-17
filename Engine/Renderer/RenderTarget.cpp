

#include "RenderTarget.hpp"
 #include "../VulkanCore/swapchain.hpp"





RenderTarget::RenderTarget(JDevice& device, JSwapchain& swapChain):
        device_app(device), swapchain_app(swapChain)        
{
    createSwapChainImageViews();
    createAttachments();
}


// RenderTarget::~RenderTarget(){
    
// }


void RenderTarget::createSwapChainImageViews() {
    swapChainImageViews_.resize(swapchain_app.getSwapChainImages().size());

    for (size_t i = 0; i < swapchain_app.getSwapChainImages().size(); ++i) {
        auto viewInfo = ImageViewCreateInfoBuilder(swapchain_app.getSwapChainImages()[i])
                        .format(swapchain_app.getSwapChainImageFormat())
                        .componentsRGBA(VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY,VK_COMPONENT_SWIZZLE_IDENTITY)
                        .getInfo();
        VkResult result =  device_app.createImageViewWithInfo(viewInfo, swapChainImageViews_[i]);
        if(result!= VK_SUCCESS){
            std::cerr << "swapchain image view creation failed at index : " << i ;
            assert(result == VK_SUCCESS && "swapchain image views create failed!");  }
        }
}


// create render target for all image views
void RenderTarget::createAttachments(){

    /* ----------------------------------*/
    /*     create depth attachment      */

    VkFormat depthFormat = device_app.findDepthFormat();
    auto imageInfo = ImageCreateInfoBuilder(swapchain_app.getSwapChainExtent().width, swapchain_app.getSwapChainExtent().height)
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

    vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);
 
    depthAttachmentInfo_ =  vk::RenderingAttachmentInfo()
      .setImageView(depthImageView_)
      .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setClearValue(clearDepth);

    // auto& depthAttachmentInfo = depthAttachmentInfo_;
    // depthAttachmentInfo.imageView  = depthImageView_ ;
    // depthAttachmentInfo.imageLayout  = vk::ImageLayout::eDepthStencilAttachmentOptimal ;
    // depthAttachmentInfo.loadOp  = vk::AttachmentLoadOp::eClear ;
    // depthAttachmentInfo.storeOp  = vk::AttachmentStoreOp::eDontCare ;
    // depthAttachmentInfo.clearValue  = clearDepth  ;

    /*                                       */
    /* --------------------------------------*/


    /* ----------------------------------*/
    /*     create color attachment      */
    //according to official tutorial, based on swap chain image to create resolved for multi-sampling
    int imageCount = swapchain_app.getSwapChainImages().size();
    for (int i = 0 ; i < imageCount ; i++ )
    {
        vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
        colorAttachmentInfo_ = vk::RenderingAttachmentInfo()
            .setImageView ( colorImageView_  )
            .setImageLayout ( vk::ImageLayout::eColorAttachmentOptimal )
            .setResolveImageLayout (vk::ImageLayout::eColorAttachmentOptimal)
            .setResolveImageView(swapChainImageViews_[i]) 
            .setResolveMode(vk::ResolveModeFlagBits::eAverage)
            .setLoadOp ( vk::AttachmentLoadOp::eClear  )
            .setStoreOp ( vk::AttachmentStoreOp::eStore  )
            .setClearValue ( clearColor  );

    }


    /*                                       */
    /* --------------------------------------*/

}



// void RenderTarget::createColorResources() {
    // VkFormat colorFormat = swapchain_app.getSwapChainImageFormat();
    // auto imageInfo = ImageCreateInfoBuilder(swapchain_app.getSwapChainExtent().width, swapchain_app.getSwapChainExtent().height)
    //                 .samples(device_app.msaaSamples())
    //                 .format(colorFormat)
    //                 .usage(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT|VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    //                 .getInfo();
    // VkResult res = device_app.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage_, colorImageMemory_);
    // assert(res == VK_SUCCESS && "failed to create color image for color resources!");

    // auto viewInfo = ImageViewCreateInfoBuilder(colorImage_)
    //                 .format(colorFormat)
    //                 .getInfo();
    // VkResult res_view = device_app.createImageViewWithInfo(viewInfo, colorImageView_);
    // assert(res_view == VK_SUCCESS && "failed to create color image view for color resources!");
    

// }
// void RenderTarget::createRenderPass() {
    // VkAttachmentDescription colorAttachment{};
    // colorAttachment.format = swapChainImageFormat_;
    // colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // colorAttachment.samples = device_app.msaaSamples();
    // colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // mutisampled image cannot be present directly

    // VkAttachmentReference colorAttachmentRef{};
    // colorAttachmentRef.attachment = 0;
    // colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // we need to add resolve attachement to present m ulti-sampled image
    // VkAttachmentDescription colorAttachmentResolve{};
    // colorAttachmentResolve.format = swapChainImageFormat_;
    // colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    // colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // VkAttachmentReference colorAttachmentResolveRef{};
    // colorAttachmentResolveRef.attachment = 2;
    // colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // VkAttachmentDescription depthAttachment{};
    // depthAttachment.format = findDepthFormat();
    // depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // depthAttachment.samples = device_app.msaaSamples();
    // depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // VkAttachmentReference depthAttachmentRef{};
    // depthAttachmentRef.attachment = 1;
    // depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

//     VkSubpassDescription subpass{};
//     subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//     subpass.colorAttachmentCount = 1;
//     subpass.pColorAttachments = &colorAttachmentRef;
//     subpass.pResolveAttachments = &colorAttachmentResolveRef;
//     subpass.pDepthStencilAttachment = &depthAttachmentRef;

//     VkSubpassDependency dependency{};
//     dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//     dependency.dstSubpass = 0;
//     //srcStageMask 外部操作中哪些pipeline阶段的完成必须字啊下游downstream开始前被确保完成
//     dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
//     //srcAccessMask 外部操作中哪些访问类型要被同步
//     dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//     // dstStageMask 在0号subpass中，要等待哪些pipeline stage完成后才能安全开始
//     dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//     //dstAccessMask 哪些类型的访问要等到srcStageMask和srcAccessMask完成后再开始。
//     dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

//     std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
//     VkRenderPassCreateInfo renderPassInfo{};
//     renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//     renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//     renderPassInfo.pAttachments = attachments.data();
//     renderPassInfo.subpassCount = 1;
//     renderPassInfo.pSubpasses = &subpass;
//     renderPassInfo.dependencyCount = 1;
//     renderPassInfo.pDependencies = &dependency;
    
//     if (vkCreateRenderPass(device_app.getDevice(), &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS) {
//         throw std::runtime_error("failed to create render pass!");
//     }
// }


// void RenderTarget::createFramebuffers() {
//     swapChainFramebuffers_.resize(swapChainImageViews_.size());

//     for (size_t i = 0; i < swapChainImageViews_.size(); i++) {
//         std::array<VkImageView, 3> attachments = { // order is very important here, need to match with above attachment description
//             colorImageView_,            
//             depthImageView_,
//             swapChainImageViews_[i],

//         };

//         VkFramebufferCreateInfo framebufferInfo{};
//         framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//         framebufferInfo.renderPass = renderPass_;
//         framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//         framebufferInfo.pAttachments = attachments.data();
//         framebufferInfo.width = swapChainExtent_.width;
//         framebufferInfo.height = swapChainExtent_.height;
//         framebufferInfo.layers = 1;

//         if (vkCreateFramebuffer(device_app.getDevice(), &framebufferInfo, nullptr, &swapChainFramebuffers_[i]) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create framebuffer!");
//         }
//     }
// }















// void RenderTarget::createSyncObjects() {
//     imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
//     renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
//     inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

//     VkSemaphoreCreateInfo semaphoreInfo{};
//     semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

//     VkFenceCreateInfo fenceInfo{};
//     fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//     fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

//     for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//         if (vkCreateSemaphore(device_app.getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS ||
//             vkCreateSemaphore(device_app.getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS ||
//             vkCreateFence(device_app.getDevice(), &fenceInfo, nullptr, &inFlightFences_[i]) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create synchronization objects for a frame!");
//         }
//     }
// }



