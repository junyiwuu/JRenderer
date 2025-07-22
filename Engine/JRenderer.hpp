
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>



#include "./VulkanCore/window.hpp"
#include "./VulkanCore/device.hpp"
#include "./VulkanCore/swapchain.hpp"
#include "./VulkanCore/utility.hpp"
#include "./VulkanCore/shaderModule.hpp"
#include "./VulkanCore/pipeline.hpp"
#include "./VulkanCore/commandBuffer.hpp"
#include "./VulkanCore/buffer.hpp"
#include "./VulkanCore/descriptor.hpp"
#include "./VulkanCore/load_texture.hpp"
#include "./VulkanCore/load_model.hpp"
#include "./Interface/JImGui.hpp"
#include "./VulkanCore/sync.hpp"
#include "./VulkanCore/global.hpp"


class JRenderer {
public:


    void run() ;
    void mainLoop();



private:
    //window
    std::unique_ptr<JWindow> window_app;
    GLFWwindow* window  = nullptr;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    //device
    std::unique_ptr<JDevice> device_app;

    //swapchain
    //one window only need one swapchain
    std::unique_ptr<JSwapchain> swapchain_app;

    //sync objects
    std::vector<std::unique_ptr<JSync>> sync_objs;

    //pipeline
    std::unique_ptr<JPipeline> pipeline_app;
    std::unique_ptr<JPipelineLayout> pipelinelayout_app;


    //commandbuffers
    std::vector<std::unique_ptr<JCommandBuffer>> commandBuffers_app;


    uint32_t currentFrame = 0;


    // const std::vector<Vertex> vertices ;
    // const std::vector<uint32_t> indices ;
    //vertex, index, uniformBuffer
    std::unique_ptr<JVertexBuffer> vertexBuffer_obj;
    std::unique_ptr<JIndexBuffer> indexBuffer_obj;
    std::vector<std::unique_ptr<JUniformBuffer>> uniformBuffer_objs;

    //descriptor
    std::unique_ptr<JDescriptorPool> descriptorPool_obj;
    std::unique_ptr<JDescriptorSetLayout> descriptorSetLayout_obj;//现在只用一种layout.如果需要不同的layout就设置多个
    std::vector<VkDescriptorSet> descriptorSets;

    //texture
    std::unique_ptr<JTexture> vikingTexture_obj;
    std::unique_ptr<JModel> vikingModel_obj;
    bool framebufferResized = false;

    //imgui
    std::unique_ptr<JImGui> imgui_obj;





    void init();
    void recreateSwapChain();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    bool drawFrame() ;





};