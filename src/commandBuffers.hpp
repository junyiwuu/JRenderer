#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "swapchain.hpp"



class JCommandBuffers{


public:


    JCommandBuffers(
           JDevice& device);
    ~JCommandBuffers();

    std::vector<VkCommandBuffer> getCommandBuffers() {return commandBuffers_;}




private:
    JDevice& device_app;
    std::vector<VkCommandBuffer> commandBuffers_;

    void createCommandBuffers(JDevice& device);















};