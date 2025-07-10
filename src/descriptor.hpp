#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

#include "buffer.hpp"



class JDescriptorPool{
public:

    JDescriptorPool(VkDevice& device, int frames);
    ~JDescriptorPool();


    JDescriptorPool(const JDescriptorPool&) = delete;
    JDescriptorPool& operator=(const JDescriptorPool&) = delete;

    VkDescriptorPool descriptorPool() {return descriptorPool_;}



private:

    VkDevice& device_;
    VkDescriptorPool descriptorPool_;

    void createDescriptorPool(VkDevice& device,  int frames);

};


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////



class JDescriptorSets{
    public:
    
        JDescriptorSets(VkDevice& device, JDescriptorPool& descriptorPool_obj, int frames, std::vector<VkBuffer> uniformBuffers);
        ~JDescriptorSets();
    
        JDescriptorSets(const JDescriptorSets&) = delete;
        JDescriptorSets& operator=(const JDescriptorSets&) = delete;
        
        std::vector<VkDescriptorSet> descriptorSets(){return descriptorSets_;}
        VkDescriptorSetLayout descriptorSetLayout() {return descriptorSetLayout_;}


    
    
    private:
    
        VkDevice& device_;
        VkDescriptorSetLayout descriptorSetLayout_;
        std::vector<VkDescriptorSet> descriptorSets_;

        void createDescriptorSets(VkDevice& device, JDescriptorPool& descriptorPool_obj,int frames,
            std::vector<VkBuffer> uniformBuffers);
    
        void createDescriptorSetLayout(VkDevice& device);
    };
    




































