#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "JRenderer.hpp"



void JRenderer::init() {
    window_app =  std::make_unique<JWindow>(WIDTH, HEIGHT, "vulkan");
    window  = window_app->getGLFWwindow();

    device_app = std::make_unique<JDevice>(*window_app);

    swapchain_app = std::make_unique<JSwapchain>(*device_app, *window_app);

    sync_objs.reserve(Global::MAX_FRAMES_IN_FLIGHT);
    for(size_t i=0 ; i<Global::MAX_FRAMES_IN_FLIGHT ; ++i){
        sync_objs.push_back(std::make_unique<JSync>(*device_app));
    }

    vikingTexture_obj = std::make_unique<JTexture>("../assets/viking_room.png", *device_app);
    vikingModel_obj = std::make_unique<JModel>("../assets/viking_room.obj");

    vertexBuffer_obj = std::make_unique<JVertexBuffer>(*device_app, vikingModel_obj->vertices(), device_app->getCommandPool(), device_app->graphicsQueue());
    indexBuffer_obj = std::make_unique<JIndexBuffer>(*device_app, vikingModel_obj->indices(), device_app->getCommandPool(), device_app->graphicsQueue());

    descriptorSetLayout_obj = JDescriptorSetLayout::Builder{*device_app}
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
        .build();
    descriptorPool_obj  = JDescriptorPool::Builder{*device_app}
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6)
        .setMaxSets(6)
        .build();

    

    uniformBuffer_objs.reserve(Global::MAX_FRAMES_IN_FLIGHT);
    descriptorSets.resize(Global::MAX_FRAMES_IN_FLIGHT);
    for(size_t i =0; i< Global::MAX_FRAMES_IN_FLIGHT; ++i){
        
        uniformBuffer_objs.emplace_back( std::make_unique<JUniformBuffer>(*device_app) );
        auto& ubo = *uniformBuffer_objs.back();
        JDescriptorWriter writer{*descriptorSetLayout_obj, *descriptorPool_obj };

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = ubo.buffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = vikingTexture_obj->textureImageView();
        imageInfo.sampler = vikingTexture_obj->textureSampler();

        if( !writer
                    .writeBuffer(0, &bufferInfo)
                    .writeImage(1, &imageInfo)
                    .build(descriptorSets[i])){
            throw std::runtime_error("failed to allocate descriptor set!");    }  
    }

    PipelineConfigInfo pipelineConfig{};
    JPipeline::defaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipelineConfig.multisampleInfo.rasterizationSamples = device_app->msaaSamples();

    VkDescriptorSetLayout setLayouts[] = {descriptorSetLayout_obj->descriptorSetLayout()};
    pipelinelayout_app = JPipelineLayout::Builder{*device_app}
                        .setDescriptorSetLayout(1, setLayouts)
                        .build();

    pipeline_app = std::make_unique<JPipeline>(*device_app, *swapchain_app,
                    "../shaders/shader.vert.spv", "../shaders/shader.frag.spv",
                    pipelinelayout_app->getPipelineLayout(), pipelineConfig);



    commandBuffers_app.reserve(Global::MAX_FRAMES_IN_FLIGHT);
    for(size_t i =0; i< Global::MAX_FRAMES_IN_FLIGHT; ++i){
        std::unique_ptr<JCommandBuffer> commandBuffer_app = std::make_unique<JCommandBuffer>(*device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        commandBuffers_app.push_back(std::move(commandBuffer_app));
    }

    // Initialize ImGui
    imgui_obj = std::make_unique<JImGui>(*device_app, *swapchain_app, window);
    
}


void JRenderer::run(){
    init();
    mainLoop();
}

void JRenderer::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Start ImGui frame
        imgui_obj->newFrame();
        
        bool frameRendered = drawFrame();
        
        // Always finish ImGui frame, even if rendering was skipped
        if (frameRendered) {
            // Normal frame completion handled in drawFrame
        } else {
            // Frame was skipped (e.g., swapchain recreation), finish ImGui anyway
            ImGui::Render(); // Finish ImGui frame without rendering
        }
        
        // End ImGui frame
        imgui_obj->endFrame();
    }

    vkDeviceWaitIdle(device_app->device());
}



//-----------------------------------------------------------------------------------


void JRenderer::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device_app->device());

    if(swapchain_app == nullptr){  //第一次初始化的时候
        swapchain_app =std::make_unique<JSwapchain>(*device_app, *window_app);
    }else { // 如果不是第一次创建，也就是resize的时候
        std::shared_ptr<JSwapchain> oldSwapChain = std::move(swapchain_app);
        swapchain_app = std::make_unique<JSwapchain>(*device_app, *window_app, oldSwapChain);

    }

}




   

void JRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {


    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType         = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView     = swapchain_app->getColorImageView();
    colorAttachment.imageLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveImageView = swapchain_app->getSwapChainImageView()[imageIndex];
    colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveMode   =  VK_RESOLVE_MODE_AVERAGE_BIT;
    colorAttachment.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue    = { {0.1f,0.1f,0.1f,1.0f} };

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView   = swapchain_app->getDepthImageView();
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.clearValue  = { {1.0f,0} };

    VkRenderingInfo renderingInfo{};

    renderingInfo.sType                   = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea             = VkRect2D{ {0,0}, {swapchain_app->getSwapChainExtent()} };
    renderingInfo.layerCount             = 1;
    renderingInfo.viewMask               = 0;
    renderingInfo.colorAttachmentCount   = 1;
    renderingInfo.pColorAttachments      = &colorAttachment;
    renderingInfo.pDepthAttachment       = &depthAttachment;      // 或 nullptr
    renderingInfo.pStencilAttachment     = nullptr;


   

    // start the command
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // layout transfer
    device_app->transitionImageLayout(commandBuffer, swapchain_app->getSwapChainImage()[imageIndex], 
            VK_IMAGE_LAYOUT_UNDEFINED , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, 1);

    device_app->transitionImageLayout(commandBuffer, swapchain_app->getColorImage(), 
            VK_IMAGE_LAYOUT_UNDEFINED , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_IMAGE_ASPECT_COLOR_BIT, 1);
    
    device_app->transitionImageLayout(commandBuffer, swapchain_app->getDepthImage(), 
            VK_IMAGE_LAYOUT_UNDEFINED , VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            VK_IMAGE_ASPECT_DEPTH_BIT, 1);



    vkCmdBeginRendering(commandBuffer, &renderingInfo);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_app->getGraphicPipeline());

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapchain_app->getSwapChainExtent().width;
        viewport.height = (float) swapchain_app->getSwapChainExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchain_app->getSwapChainExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_app->getGraphicPipeline());
        VkBuffer vertexBuffers[] = {vertexBuffer_obj->baseBuffer.buffer()};
        VkDeviceSize offsets[] = {0};
        //binding
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer,indexBuffer_obj->baseBuffer.buffer(), 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout_app->getPipelineLayout(),0,1,
                    &descriptorSets[currentFrame], 0, nullptr );

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(vikingModel_obj->indices().size()), 1, 0, 0, 0);
        // vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

        imgui_obj->render(commandBuffer);

    vkCmdEndRendering(commandBuffer);
 //  ----- setup ui
    // VkRenderingAttachmentInfo UIcolorAttachment{};
    // UIcolorAttachment.sType         = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    // UIcolorAttachment.imageView     = swapchain_app->getSwapChainImageView()[imageIndex];
    // UIcolorAttachment.imageLayout   = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // UIcolorAttachment.loadOp        = VK_ATTACHMENT_LOAD_OP_LOAD;
    // UIcolorAttachment.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
    

    // VkRenderingInfo UIrenderingInfo{};
    // UIrenderingInfo.sType                   = VK_STRUCTURE_TYPE_RENDERING_INFO;
    // UIrenderingInfo.renderArea             = { {0,0}, swapchain_app->getSwapChainExtent() };
    // UIrenderingInfo.layerCount             = 1;
    // UIrenderingInfo.colorAttachmentCount   = 1;
    // UIrenderingInfo.pColorAttachments      = &UIcolorAttachment;
    // UIrenderingInfo.pDepthAttachment       = nullptr;   
    //start ui pass
    // vkCmdBeginRendering(commandBuffer, &UIrenderingInfo);
    // imgui_obj->render(commandBuffer);
    // ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    // vkCmdEndRendering(commandBuffer);


    device_app->transitionImageLayout(commandBuffer, swapchain_app->getSwapChainImage()[imageIndex], 
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 
        VK_IMAGE_ASPECT_COLOR_BIT, 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}



bool JRenderer::drawFrame() {
    //阻塞CPU，知道in flight fence关联的那次vk queue submit都执行完毕后，把fence设置为signaled
    //让CPU卡住等GPU完成，因为需要等上一帧的命令执行完才能再重用buffer, image等资源
    vkWaitForFences(device_app->device(), 1, &sync_objs[currentFrame]->inFlightFence, VK_TRUE, UINT64_MAX);



    uint32_t imageIndex;  
    // signal the semaphore，初始化的时候是unsignaled的状态，执行这条后，semaphore从unsignal变成了signal
    VkResult result = vkAcquireNextImageKHR(device_app->device(), swapchain_app->swapChain(), 
                UINT64_MAX, sync_objs[currentFrame]->imageAvailableSemaphore, 
                VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return false;  // Frame was skipped
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }


    //把fence从signal变成unsignal
    vkResetFences(device_app->device(), 1,  &sync_objs[currentFrame]->inFlightFence);

    
    //--------- update uniform buffer
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float updateUniformBuffer_time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), updateUniformBuffer_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapchain_app->getSwapChainExtent().width / (float)swapchain_app->getSwapChainExtent().height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffer_objs[currentFrame]->bufferMapped(), &ubo, sizeof(ubo) );
    commandBuffers_app[currentFrame]->reset();
    recordCommandBuffer(commandBuffers_app[currentFrame]->getCommandBuffer(), imageIndex);
    //---------------------------------------




    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //wait for this semaphore signaled, then execute 
    VkSemaphore waitSemaphores[] = {sync_objs[currentFrame]->imageAvailableSemaphore}; //通过之前的acquire next，这里期待是得到的signaled semaphore
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &(commandBuffers_app[currentFrame]->getCommandBuffer());
    VkSemaphore signalSemaphores[] = {sync_objs[currentFrame]->renderFinishedSemaphore};//当前为unsignaled
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // 做queue submit的时候fence必须是Unsignal的状态
    if (vkQueueSubmit(device_app->graphicsQueue(), 1, &submitInfo,  sync_objs[currentFrame]->inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }//GPU执行命令，完成后fence自动变成signaled.
    //也自动把render finished sempahore变成signaled
    //GPU消费semaphore，所以这里的wait semaphore自动从signal 消费成unsignal

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {swapchain_app->swapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(device_app->presentQueue()  , &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % Global::MAX_FRAMES_IN_FLIGHT;
    return true;  // Frame was successfully rendered
}




