#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "../AppContext.hpp"
#include "../device.hpp"
#include "../global.hpp"
#include "../buffer.hpp"
#include "../commandBuffer.hpp"
#include "../load_texture.hpp"
#include "../descriptor.hpp"
#include "../pipeline.hpp"


struct UISettings {
	bool displayModels = true;
	bool displayLogos = true;
	bool displayBackground = true;
	bool animateLight = false;
	float lightSpeed = 0.25f;
	std::array<float, 50> frameTimes{};
	float frameTimeMin = 9999.0f, frameTimeMax = 0.0f;
	float lightTimer = 0.0f;
} ;
extern UISettings uiSettings;


static void check_vk_result(VkResult err)
{
    if (err == VK_SUCCESS)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

class JGui{

public:

    JGui(AppContext* appContext, JDevice& device);
    ~JGui();

    struct PushConstBlock{
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstBlock;


    void init(float width, float height);
    // void initResources(VkRenderPass renderPass, VkQueue queue, const std::string& shadersPath);
    void initResources(VkRenderPass renderPass, GLFWwindow* window);
	void newFrame(AppContext* appContext, bool updateFrameGraph);
    void setStyle(uint32_t index);
    // void newFrame(device, bool updateFrameGraph);
    void updateBuffers();
    void drawFrame(VkCommandBuffer commandBuffer);


private:
    JDevice& device_app;
    AppContext* appContext;

    // VkImage fontImage_ = VK_NULL_HANDLE;
    // VkImageView fontImageView_ = VK_NULL_HANDLE;
    // VkDeviceMemory fontMemory_ = VK_NULL_HANDLE;
    // VkSampler sampler_;
    VkPipelineCache pipelineCache;
    // VkPipeline pipeline_;
    // // std::unique_ptr<JPipelineLayout> pipelineLayout_obj;
    // std::vector<VkDescriptorSetLayout> descriptorSetLayouts_collect;
    std::unique_ptr<JDescriptorPool> descriptorPool_obj;
    // std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_obj;
    // VkDescriptorSet descriptorSets;

    // tools::Buffer vertexBuffer;
    // tools::Buffer indexBuffer;
	// int32_t vertexCount = 0;
	// int32_t indexCount = 0;


    int selectedStyle = 0;

    ImGuiStyle vulkanStyle;

   
};