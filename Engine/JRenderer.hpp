#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

class JWindow;
class JDevice;
class JPipeline;
class JPipelineLayout;
class JSwapchain;
class JCommandBuffer;
class JVertexBuffer;
class JIndexBuffer;
class JUniformBuffer;
class JDescriptorPool;
class JDescriptorSetLayout;
class JTexture;
class JModel;




class JRenderer {
public:

    void init() ;

    void mainLoop();


private:
   

    //window
    std::unique_ptr<JWindow> window_app;
    GLFWwindow* window  = nullptr;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    //device
    std::unique_ptr<JDevice> device_app;

    // VkDevice device;
    // VkInstance instance;
    // VkDebugUtilsMessengerEXT debugMessenger;
    // VkSurfaceKHR surface;

    // VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    // VkQueue graphicsQueue;
    // VkQueue presentQueue;
    // VkCommandPool commandPool;

    //swapchain
    //one window only need one swapchain
    std::unique_ptr<JSwapchain> swapchain_app;
    VkSwapchainKHR swapChain;

    //pipeline
    std::unique_ptr<JPipeline> pipeline_app;
    std::unique_ptr<JPipelineLayout> pipelinelayout_app;
    VkPipeline graphicPipeline;


    

    uint32_t currentFrame = 0;


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

    // std::unique_ptr<AppContext> appContext;

    // //interface
    // std::unique_ptr<JGui> gui;






};