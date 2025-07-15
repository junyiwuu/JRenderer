#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "device.hpp"
#include "global.hpp"
#include "buffer.hpp"
#include "commandBuffer.hpp"
#include "load_texture.hpp"
#include "descriptor.hpp"
#include "pipeline.hpp"


class JGui{

public:

    JGui(JDevice& device);
    ~JGui();

    struct PushConstBlock{
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstBlock;





private:
    JDevice& device_app;
    VkImage fontImage_ = VK_NULL_HANDLE;
    VkImageView fontImageView_ = VK_NULL_HANDLE;
    VkDeviceMemory fontMemory_ = VK_NULL_HANDLE;
    VkSampler sampler_;
    VkPipelineCache pipelineCache;


    std::unique_ptr<JDescriptorPool> descriptorPool_obj;
    std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_obj;
    VkDescriptorSet descriptorSets;
    
    void initResources(VkRenderPass renderPass, VkQueue queue, const std::string& shadersPath);
	



};