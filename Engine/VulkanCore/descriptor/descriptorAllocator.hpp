#pragma once
#include <vulkan/vulkan.hpp>

class JDevice;



// ---------------------------------

class JDescriptorAllocator{



public:

    JDescriptorAllocator( JDevice& device, 
                const std::vector<VkDescriptorPoolSize>& p_poolSizes,
                uint32_t initialSetsCount = 10);
    ~JDescriptorAllocator();

    //function
    VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout descriptorSetLayout);

    //getter
    VkDescriptorPool getDescriptorPool() const {
        printf("DEBUG: getDescriptorPool returning: %p\n", (void*)currentPool_);
        return currentPool_;
    }




private:
    JDevice&                            device_app;


    VkDescriptorPool                    currentPool_;
    std::vector<VkDescriptorPool>       usedPool_;

    uint32_t                            maxSetsCounter_;

    std::vector<VkDescriptorPoolSize>   p_poolSizes_{};  // {descriptor type, count}



    VkDescriptorPool createPool(uint32_t maxSetsCounter_);






};