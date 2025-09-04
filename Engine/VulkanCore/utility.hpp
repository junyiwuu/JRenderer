#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "stb_image.h"
#include "stb_image_write.h"
#include <ktx.h>

#include "global.hpp"

namespace util{




std::vector<char> readFile(const std::string& filename);



inline VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool){
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    return commandBuffer;
}

inline void endSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, VkCommandPool commandPool ,VkQueue queue){
    
    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1 , &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    // vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}





inline void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size,
            VkDevice device, VkCommandPool commandPool, VkQueue queue){
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(device, commandPool );
    
    //process
    VkBufferCopy copyRegion{};
    // copyRegion.srcOffset = 0;
    // copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    //end command buffer
    endSingleTimeCommands(device, commandBuffer, commandPool, queue);
}




uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice); 




























};


















