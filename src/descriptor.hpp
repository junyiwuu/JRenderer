#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <memory>

#include "buffer.hpp"


//descriptor ppol需要知道要用的descriptor sets的数量和descriptor有多少个。但是不涉及descriptor和descriptor set是否对的上
class JDescriptorPool{
public:
    class Builder{
        public:
            Builder(JDevice& device): device_app(device) {}
            Builder& reservePoolDescriptors(VkDescriptorType descriptorType, uint32_t descriptorCount);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags poolCreateFlags);
            Builder& setMaxSets(uint32_t descriptorCount);
            std::unique_ptr<JDescriptorPool> build() const;

        private:    
            JDevice& device_app;
            std::vector<VkDescriptorPoolSize> b_poolSizes{};  // push back {type, count}
            VkDescriptorPoolCreateFlags b_poolCreateFlags = 0;
            uint32_t b_maxDescriptorSets = 1000;
    };



    JDescriptorPool(JDevice& device_app, uint32_t maxDescriptorSets, VkDescriptorPoolCreateFlags poolCreateFlags,
        const std::vector<VkDescriptorPoolSize> &poolSize);
    ~JDescriptorPool();
    JDescriptorPool(const JDescriptorPool&) = delete;
    JDescriptorPool& operator=(const JDescriptorPool&) = delete;

    VkDescriptorPool descriptorPool() {return descriptorPool_;}
    bool allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet);

    // bool allocateDescriptorSets(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const;


private:

    JDevice& device_app;
    VkDescriptorPool descriptorPool_;

    friend class JDescriptorWriter;
};


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////



class JDescriptorSetLayout{

public:
    class Builder{
        public:
            Builder(JDevice& device): device_app(device) {}

            Builder& addBinding(uint32_t binding, VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags, uint32_t descriptorCount);
            std::unique_ptr<JDescriptorSetLayout> build() const;

        private:
            JDevice& device_app;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };


    JDescriptorSetLayout(JDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
    ~JDescriptorSetLayout();
          
    JDescriptorSetLayout(const JDescriptorSetLayout &) = delete;
    JDescriptorSetLayout &operator=(const JDescriptorSetLayout &) = delete;
  

    VkDescriptorSetLayout descriptorSetLayout() const {return descriptorSetLayout_;}


private:
    JDevice& device_app;
    VkDescriptorSetLayout descriptorSetLayout_;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings_;

    friend class JDescriptorWriter;
};


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////



class JDescriptorWriter{
public:

    JDescriptorWriter(JDescriptorSetLayout& descriptorSetLayout, JDescriptorPool& descriptorPool);
    JDescriptorWriter(const JDescriptorWriter&) = delete;
    JDescriptorWriter& operator=(const JDescriptorWriter&) = delete;


    JDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
    JDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

    bool build(VkDescriptorSet& set);
    bool overwrite(VkDescriptorSet& set);
    
    
private:

    JDescriptorSetLayout &descriptorSetLayout_;
    JDescriptorPool& descriptorPool_;
    std::vector<VkWriteDescriptorSet> descriptorWrites_;


    std::vector<VkDescriptorSet> descriptorSets_;

    void createDescriptorSets(VkDevice& device, JDescriptorPool& descriptorPool_obj,int frames,
        std::vector<VkBuffer> uniformBuffers);

    
    };
    








