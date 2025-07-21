#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

#include "global.hpp"
class JDevice;

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