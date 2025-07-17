

#include "window.hpp"
#include "device.hpp"
#include "swapchain.hpp"
#include "utility.hpp"
#include "shaderModule.hpp"
#include "pipeline.hpp"
#include "commandBuffer.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"
#include "load_texture.hpp"
#include "load_model.hpp"
#include "Interface/gui.hpp"
#include "AppContext.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "JRenderer.hpp"




void JRenderer::init() {
    window_app =  std::make_unique<JWindow>(WIDTH, HEIGHT, "vulkan");
    window  = window_app->getGLFWwindow();

    device_app = std::make_unique<JDevice>(*window_app);
    device = device_app->device();
    surface = device_app->surface();
    graphicsQueue = device_app->graphicsQueue();
    presentQueue = device_app->presentQueue();
    physicalDevice = device_app->physicalDevice();
    commandPool = device_app->getCommandPool();

    swapchain_app = std::make_unique<JSwapchain>(*device_app, *window_app);
    swapChain = swapchain_app->swapChain();

    vikingTexture_obj = std::make_unique<JTexture>("../assets/viking_room.png", *device_app);
    vikingModel_obj = std::make_unique<JModel>("../assets/viking_room.obj");

    vertexBuffer_obj = std::make_unique<JVertexBuffer>(*device_app, vikingModel_obj->vertices(), commandPool, graphicsQueue);
    indexBuffer_obj = std::make_unique<JIndexBuffer>(*device_app, vikingModel_obj->indices(), commandPool, graphicsQueue);

    descriptorSetLayout_obj = JDescriptorSetLayout::Builder{*device_app}
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1)
        .build();
    descriptorPool_obj  = JDescriptorPool::Builder{*device_app}
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6)
        .setMaxSets(6)
        .build();

    

    uniformBuffer_objs.reserve(JSwapchain::MAX_FRAMES_IN_FLIGHT);
    descriptorSets.resize(JSwapchain::MAX_FRAMES_IN_FLIGHT);
    for(size_t i =0; i< JSwapchain::MAX_FRAMES_IN_FLIGHT; ++i){
        
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
    pipelineConfig.renderPass = swapchain_app->renderPass();
    pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipelineConfig.multisampleInfo.rasterizationSamples = device_app->msaaSamples();

    VkDescriptorSetLayout setLayouts[] = {descriptorSetLayout_obj->descriptorSetLayout()};
    pipelinelayout_app = JPipelineLayout::Builder{*device_app}
                        .setDescriptorSetLayout(1, setLayouts)
                        .build();

    pipeline_app = std::make_unique<JPipeline>(*device_app, 
                    "../shaders/shader.vert.spv", "../shaders/shader.frag.spv",
                    pipelinelayout_app->getPipelineLayout(), pipelineConfig);
    graphicPipeline = pipeline_app->getGraphicPipeline();



    commandBuffers_app.reserve(JSwapchain::MAX_FRAMES_IN_FLIGHT);
    for(size_t i =0; i< JSwapchain::MAX_FRAMES_IN_FLIGHT; ++i){
        std::unique_ptr<JCommandBuffer> commandBuffer_app = std::make_unique<JCommandBuffer>(*device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        commandBuffers_app.push_back(std::move(commandBuffer_app));
    }
    
    // appContext = std::make_unique<AppContext>();

    // gui = std::make_unique<JGui>(appContext.get(), *device_app);

    // //     // 2) 建立 ImGui 上下文，传入窗口尺寸
    // gui->init((float)WIDTH, (float)HEIGHT);

    // // // 3) 创建字体贴图、pipeline、descriptor，绑定到你的 renderPass
    // gui->initResources(swapchain_app->renderPass(), window);

   
}


void JRenderer::mainLoop() {
    init();
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);
}

    
//-----------------------------------------------------------------------------------


    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }
    
        vkDeviceWaitIdle(device);
    
        if(swapchain_app == nullptr){  //第一次初始化的时候
            swapchain_app =std::make_unique<JSwapchain>(*device_app, *window_app);
        }else { // 如果不是第一次创建，也就是resize的时候
            std::shared_ptr<JSwapchain> oldSwapChain = std::move(swapchain_app);
            swapchain_app = std::make_unique<JSwapchain>(*device_app, *window_app, oldSwapChain);
        }
    }
    







   

};




