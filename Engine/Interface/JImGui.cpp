#include "JImGui.hpp"
#include "../VulkanCore/device.hpp"
#include "../VulkanCore/descriptor.hpp"
#include "../VulkanCore/swapchain.hpp"

#include <stdexcept>
#include <iostream>

JImGui::JImGui(JDevice& device, JSwapchain& swapchain, GLFWwindow* window )
    : device_app(device), window_ptr(window), swapchain_app(swapchain){
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    setupStyle();
    createDescriptorPool();

    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{};
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    VkFormat colorFormats[] = {swapchain_app.getSwapChainImageFormat()};
    pipelineRenderingCreateInfo.pColorAttachmentFormats = colorFormats; 
    pipelineRenderingCreateInfo.depthAttachmentFormat = device_app.findDepthFormat();
    pipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window_ptr, true);
    
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = device_app.getInstance();
    init_info.PhysicalDevice = device_app.physicalDevice();
    init_info.Device = device_app.device();
    init_info.QueueFamily = device_app.findPhysicalQueueFamilies().graphicsFamily.value();
    init_info.Queue = device_app.graphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptorPool_obj->descriptorPool();
    init_info.MinImageCount = swapchain_app.minImageCount_;
    init_info.ImageCount = Global::MAX_FRAMES_IN_FLIGHT;
    init_info.MSAASamples = device_app.msaaSamples();
    init_info.UseDynamicRendering = true;
    init_info.PipelineRenderingCreateInfo = pipelineRenderingCreateInfo;
    init_info.CheckVkResultFn = checkVkResult;
    
    ImGui_ImplVulkan_Init(&init_info);
    uiSettings = UIsettings();
}

JImGui::~JImGui() {
    // Wait for all operations to complete before destroying resources
    vkDeviceWaitIdle(device_app.device());
    
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void JImGui::setupStyle() {
    // Setup Dark style
    ImGui::StyleColorsDark();
    
    // Customize colors
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    
    colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.9f);
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
}

void JImGui::createDescriptorPool() {
    descriptorPool_obj = JDescriptorPool::Builder(device_app)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
        .setMaxSets(1000)
        .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
        .build();
}

void JImGui::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create a simple demo window
    static bool show_demo_window = true;
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    //checkbox
    ImGui::Begin("Render Settings");
    ImGui::Checkbox("Cull backface",  &uiSettings.cullBackFace);
    ImGui::End();

    // Create a simple debug window
    ImGui::Begin("Debug Info");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void JImGui::render(VkCommandBuffer commandBuffer) {
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void JImGui::endFrame() {
    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void JImGui::checkVkResult(VkResult err) {
    if (err == VK_SUCCESS) return;
    std::cerr << "[ImGui Vulkan] Error: VkResult = " << err << std::endl;
    if (err < 0) {
        throw std::runtime_error("ImGui Vulkan error");
    }
}