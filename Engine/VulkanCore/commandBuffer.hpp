#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "swapchain.hpp"
#include "global.hpp"



class JCommandBuffer{


public:


    JCommandBuffer(  JDevice& device,  vk::CommandBufferLevel level);
    ~JCommandBuffer();

    VkCommandBuffer& getCommandBuffer() {return commandBuffer_;}

    void beginSingleTimeCommands();
    void endSingleTimeCommands(VkQueue queue);

    void beginRecording();
    void endRecording();


private:
    JDevice& device_app;

    VkCommandBuffer commandBuffer_;
    vk::CommandBuffer commandBuffer_v;

    void createCommandBuffer(JDevice& device,  vk::CommandBufferLevel level);


    bool cmdBuf_renderingNow;


};