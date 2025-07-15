#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "swapchain.hpp"
#include "global.hpp"



class JCommandBuffer{


public:


    JCommandBuffer(
           JDevice& device,  VkCommandBufferLevel level);
    ~JCommandBuffer();

    VkCommandBuffer& getCommandBuffer() {return commandBuffer_;}

    void beginSingleTimeCommands();
    void endSingleTimeCommands(VkQueue queue);
    


private:
    JDevice& device_app;
    VkCommandBuffer commandBuffer_;

    void createCommandBuffer(JDevice& device,  VkCommandBufferLevel level);





};