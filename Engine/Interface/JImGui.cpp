#include "JImGui.hpp"
#include "../VulkanCore/device.hpp"
#include "../VulkanCore/descriptor/descriptor.hpp"
#include "../VulkanCore/swapchain.hpp"


#include <stdexcept>
#include <iostream>

JImGui::JImGui(JDevice& device, const JSwapchain& swapchain, GLFWwindow* window , UI::UISettings& uiSettings)
    : device_app(device), window_ptr(window), swapchain_app(swapchain), 
    texture_viewTest("../assets/cat.jpg", device), uiSettings(uiSettings)
{
    // UIsettings::UserCam userCam
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

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
    // We install our own GLFW callbacks and forward to ImGui backend, so set install_callbacks=false
    ImGui_ImplGlfw_InitForVulkan(window_ptr, false);
    
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
    // uiSettings = UIsettings();

    VkDescriptorSet texture_ds = ImGui_ImplVulkan_AddTexture(
        texture_viewTest.textureSampler(),
        texture_viewTest.textureImageView(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
    texID = (ImTextureID)( (uintptr_t)texture_ds );
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

// Basic slider syntax:
// // Float slider
// ImGui::SliderFloat("Roughness", &roughnessValue, 0.0f, 1.0f);

// // Color picker  
// ImGui::ColorEdit3("Base Color", baseColorArray);

// // Checkbox for toggles
// ImGui::Checkbox("Use Roughness Texture", &useRoughnessTexture);

void JImGui::newFrame() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create a simple demo window
    // static bool show_demo_window = true;
    // if (show_demo_window) {
    //     ImGui::ShowDemoWindow(&show_demo_window);
    // }

    //checkbox
    ImGui::Begin("Render Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::RadioButton("Arcball Camera",  
        reinterpret_cast<int*>(&uiSettings.userCam),
        static_cast<int>(UI::UserCam::ArcballCamera));

    ImGui::RadioButton("First Person Camera",  
        reinterpret_cast<int*>(&uiSettings.userCam),
        static_cast<int>(UI::UserCam::FirstPersonCamera));
    
    // use ## to make the same text unique
        //base color
    ImGui::Separator();
    ImGui::Text("Base Color");
    const char* BaseColButton = uiSettings.inputAlbedoPath ? "Use Slider##albedo": "Input Path##albedo";
    if(ImGui::Button(BaseColButton)){
        uiSettings.inputAlbedoPath = !uiSettings.inputAlbedoPath;    }

    if(uiSettings.inputAlbedoPath){
        ImGui::InputText("File Path##albedo ", uiSettings.albedoTexPath, sizeof(uiSettings.albedoTexPath)); 
    }else{
        ImGui::ColorEdit3("Color Picker##albedo", uiSettings.baseColor );
    }


    //roughness
    ImGui::Separator();
    ImGui::Text("Roughness");
    const char* RoughButton = uiSettings.inputRoughnessPath ? "Use Slider##rough" : "Input Path##rough";
    if(ImGui::Button(RoughButton)){
        uiSettings.inputRoughnessPath = !uiSettings.inputRoughnessPath;    }

    if(uiSettings.inputRoughnessPath){
        ImGui::InputText("File Path##rough", uiSettings.roughnessTexPath, sizeof(uiSettings.roughnessTexPath));
    }else{
        ImGui::SliderFloat("Float##rough", &uiSettings.roughness, 0.f, 1.0f);
    }


    //metallic
    ImGui::Separator();
    ImGui::Text("Metallic");
    const char* MetalButton = uiSettings.inputMetallicPath ? "Use Slider##metallic" : "Input Path##metallic";
    if(ImGui::Button(MetalButton)){
        uiSettings.inputMetallicPath = !uiSettings.inputMetallicPath;    }

    if(uiSettings.inputMetallicPath){
        ImGui::InputText("File Path##metallic", uiSettings.metallicTexPath, sizeof(uiSettings.metallicTexPath));
    }else{
        ImGui::SliderFloat("Float##metallic", &uiSettings.metallic, 0.f, 1.f);
    }
    

    //norma
    ImGui::Separator();
    ImGui::Text("Normal");
    const char* NormalButton = uiSettings.inputNormalPath ? "Default Normal##normal" : "Input Path##normal";
    if(ImGui::Button(NormalButton)){
        uiSettings.inputNormalPath = !uiSettings.inputNormalPath;    }

    if(uiSettings.inputNormalPath){
        ImGui::InputText("File Path##normal", uiSettings.normalTexPath, sizeof(uiSettings.normalTexPath));
    }else{
        // ImGui::SliderFloat("Float", &uiSettings.roughness, 0.f, 1.0f);
    }
    ImGui::End();

    


    ImGui::Begin("Texture Viewer", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Image(texID, ImVec2(texture_viewTest.getTextureWidth(), texture_viewTest.getTextureHeight()));
    ImGui::End();



    // Create a simple debug window
    ImGui::Begin("Debug Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
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
