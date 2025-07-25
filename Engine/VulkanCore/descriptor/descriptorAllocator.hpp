#pragma once
#include <vulkan/vulkan.hpp>

class JDevice;



// ---------------------------------

class JDescriptorAllocator{



public:

    JDescriptorAllocator(JDevice& device);
    ~JDescriptorAllocator();

    VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout layout);















private:
    JDevice& device_app;


    VkDescriptorPool currentPool_;
    std::vector<VkDescriptorPool> usedPool_;






    VkDescriptorPool createPool();


















};