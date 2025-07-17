#include "gui.hpp"
UISettings uiSettings{};



JGui::JGui(AppContext* appContext, JDevice& device ):
        device_app(device) , appContext(appContext)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.0f;
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.0f);
}


JGui::~JGui(){
    ImGui::DestroyContext();
    // vertexBuffer.destroy();
    // indexBuffer.destroy();
    // vkDestroyImage(device_app.device(), fontImage_, nullptr);
    // vkDestroyImageView(device_app.device(), fontImageView_, nullptr);
    // vkFreeMemory(device_app.device(), fontMemory_, nullptr);
    // vkDestroySampler(device_app.device(), sampler_, nullptr);//
    vkDestroyPipelineCache(device_app.device(), pipelineCache, nullptr);//
    // vkDestroyPipeline(device_app.device(), pipeline_, nullptr);//

}

void JGui::init(float width, float height)
{


    // Color scheme
    vulkanStyle = ImGui::GetStyle();
    vulkanStyle.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    vulkanStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    vulkanStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    vulkanStyle.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    vulkanStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

    setStyle(0);
    
    // Dimensions
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   

    io.DisplaySize = ImVec2(width, height);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

}

void JGui::setStyle(uint32_t index)
{
    switch (index)
    {
    case 0:
    {
        ImGuiStyle& style = ImGui::GetStyle();
        style = vulkanStyle;
        break;
    }
    case 1:
        ImGui::StyleColorsClassic();
        break;
    case 2:
        ImGui::StyleColorsDark();
        break;
    case 3:
        ImGui::StyleColorsLight();
        break;
    }
   
}

void JGui::initResources(VkRenderPass renderPass, GLFWwindow* window)
{
    // ImGuiIO& io = ImGui::GetIO();

    // // create font texture
    // unsigned char* fontData;
    // int texWidth, texHeight;
    // io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    // printf("fontData pointer: %p\n", fontData);
    // printf("Texture width: %d\n", texWidth);
    // printf("Texture height: %d\n", texHeight);

    // VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char); // because of 4 channels

    // auto imageInfo = ImageCreateInfoBuilder(texWidth, texHeight)
    //                 .format(VK_FORMAT_R8G8B8A8_UNORM)
    //                 .usage(VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT)
    //                 .getInfo();
    // VkResult res_img = device_app.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, fontImage_, fontMemory_);
    // assert(res_img == VK_SUCCESS && "failed to create UI font image!");

    // auto viewInfo = ImageViewCreateInfoBuilder(fontImage_)
    //                 .format(VK_FORMAT_R8G8B8A8_UNORM)
    //                 .getInfo();
    // VkResult res_view = device_app.createImageViewWithInfo(viewInfo, fontImageView_);
    // assert(res_view == VK_SUCCESS && "failed to create UI font image view!");

    
    // JBuffer stagingBuffer(device_app, uploadSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    //                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    // stagingBuffer.stagingAction(fontData);

    // // copy buffer data to font image
    // JCommandBuffer copyBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    // copyBuffer.beginSingleTimeCommands();
    // device_app.transitionImageLayout(copyBuffer.getCommandBuffer(), fontImage_, 
    //                 VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
    //                 VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
    //                 VK_IMAGE_ASPECT_COLOR_BIT, 1);
    // JTexture::copyBufferToImage(copyBuffer.getCommandBuffer(), stagingBuffer.buffer(), fontImage_, texWidth, texHeight);
    // device_app.transitionImageLayout(copyBuffer.getCommandBuffer(), fontImage_, 
    //                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
    //                 VK_IMAGE_ASPECT_COLOR_BIT, 1);
    // copyBuffer.endSingleTimeCommands(device_app.graphicsQueue());

    // // font texture sampler
    // auto fontSamplerInfo = SamplerCreateInfoBuilder()
    //                         .addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
    //                         .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
    //                         .getInfo();
    // vkCreateSampler(device_app.device(), &fontSamplerInfo, nullptr, &sampler_);

    // //descriptor
    // descriptorSetLayout_obj = JDescriptorSetLayout::Builder(device_app)
    //                         .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
    //                         .build();                        
    descriptorPool_obj = JDescriptorPool::Builder(device_app)
                        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1)
                        .setMaxSets(2)
                        .build();
    // JDescriptorWriter writer(*descriptorSetLayout_obj, *descriptorPool_obj);
    // VkDescriptorImageInfo des_imgInfo{};
    // des_imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    // des_imgInfo.imageView = fontImageView_;
    // des_imgInfo.sampler = sampler_;
    // if(!writer.writeImage(0, &des_imgInfo).build(descriptorSets)){
    //     throw std::runtime_error("failed to build font image descriptor set!");}

    // pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK_RESULT(vkCreatePipelineCache(device_app.device(), &pipelineCacheCreateInfo, nullptr, &pipelineCache));

//     // push constant for ui render

//     VkPushConstantRange pushConstRange{};
//     pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//     pushConstRange.offset = 0;
//     pushConstRange.size = sizeof(PushConstBlock);
    
//     //pipeline
//     descriptorSetLayouts_collect = {descriptorSetLayout_obj->descriptorSetLayout()};
//     pipelineLayout_obj = JPipelineLayout::Builder(device_app)
//                                 .setDescriptorSetLayout(descriptorSetLayouts_collect.size(), descriptorSetLayouts_collect.data())
//                                 .setPushConstRanges(1, &pushConstRange)
//                                 .build();
//     PipelineConfigInfo pipelineCfg{};
//     JPipeline::defaultPipelineConfigInfo(pipelineCfg);
//     pipelineCfg.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
//     pipelineCfg.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
//     // color blend
//     pipelineCfg.colorBlendAttachment.blendEnable = VK_TRUE;
//     pipelineCfg.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//     pipelineCfg.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//     pipelineCfg.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
//     pipelineCfg.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//     pipelineCfg.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//     pipelineCfg.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

//     pipelineCfg.depthStencilInfo.depthTestEnable = VK_FALSE;
//     pipelineCfg.depthStencilInfo.depthWriteEnable = VK_FALSE;
//     pipelineCfg.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

//     pipelineCfg.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;
//     /////should be changed later, because dont want imgui using same sample as the main render

//     pipelineCfg.renderPass = renderPass;



//     ///////////////////////////////////////////////////////////////////////
//     ///////////////////////////  SHADER ///////////////////////////////////
//     std::vector<VkVertexInputBindingDescription> vertexInputBindings;
//     vertexInputBindings.resize(1);
//     vertexInputBindings[0].binding = 0;
//     vertexInputBindings[0].stride = sizeof(ImDrawVert);
//     vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

//     std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;
//     vertexInputAttributes.resize(3);
//     vertexInputAttributes[0] = { 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)};
//     vertexInputAttributes[1] = { 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv) };
//     vertexInputAttributes[2] = { 2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)};
    
//     auto vertShaderCode = util::readFile(shadersPath + "imgui/ui.vert.spv");
//     auto fragShaderCode = util::readFile(shadersPath + "imgui/ui.frag.spv");
//     JShaderModule vertShaderModule_obj{device_app.device(), vertShaderCode};
//     VkShaderModule vertShaderModule = vertShaderModule_obj.getShaderModule();
//     JShaderModule fragShaderModule_obj{device_app.device(), fragShaderCode};
//     VkShaderModule fragShaderModule = fragShaderModule_obj.getShaderModule();

//     VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
//     vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//     vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//     vertShaderStageInfo.module = vertShaderModule;
//     vertShaderStageInfo.pName = "main";

//     VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
//     fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//     fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//     fragShaderStageInfo.module = fragShaderModule;
//     fragShaderStageInfo.pName = "main";

//     VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

//     VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
//     vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//     vertexInputInfo.vertexBindingDescriptionCount =  static_cast<uint32_t>(vertexInputBindings.size());
//     vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
//     vertexInputInfo.pVertexBindingDescriptions = vertexInputBindings.data();
//     vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributes.data();
//     ////////////////////////////  SHADER ///////////////////////////////////
//     ////////////////////////////////////////////////////////////////////////

//     VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
//     pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//     pipelineCreateInfo.stageCount = 2;
//     pipelineCreateInfo.pStages = shaderStages;
//     pipelineCreateInfo.pVertexInputState = &vertexInputInfo;

//     pipelineCreateInfo.pInputAssemblyState = &pipelineCfg.inputAssemblyInfo;
//     pipelineCreateInfo.pViewportState = &pipelineCfg.viewportInfo;
//     pipelineCreateInfo.pRasterizationState = &pipelineCfg.rasterizationInfo;
//     pipelineCreateInfo.pMultisampleState = &pipelineCfg.multisampleInfo;
//     pipelineCreateInfo.pColorBlendState = &pipelineCfg.colorBlendInfo;
//     pipelineCreateInfo.pDynamicState = &pipelineCfg.dynamicStateInfo;
//     pipelineCreateInfo.pDepthStencilState = &pipelineCfg.depthStencilInfo;
//     pipelineCreateInfo.layout = pipelineLayout_obj->getPipelineLayout();
//     pipelineCreateInfo.renderPass = pipelineCfg.renderPass;
//     pipelineCreateInfo.subpass = pipelineCfg.subpass;

//     if (vkCreateGraphicsPipelines(device_app.device(), pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline_) != VK_SUCCESS) {
//         throw std::runtime_error("failed to create graphics pipeline!");
// }


ImGui_ImplGlfw_InitForVulkan(window, true);
ImGui_ImplVulkan_InitInfo init_info = {};
init_info.Instance = device_app.instance();
init_info.PhysicalDevice = device_app.physicalDevice();
init_info.Device = device_app.device();
init_info.QueueFamily = device_app.findPhysicalQueueFamilies().graphicsFamily.value();
init_info.Queue = device_app.graphicsQueue();
init_info.PipelineCache = pipelineCache;
init_info.DescriptorPool = descriptorPool_obj->descriptorPool();
init_info.RenderPass = renderPass;
init_info.Subpass = 0;
init_info.MinImageCount = 2;
init_info.ImageCount = 2;   // should be swapchain image, not yet
init_info.MSAASamples = VK_SAMPLE_COUNT_8_BIT;
init_info.Allocator = nullptr;  // gpt suggest nullptr for now
init_info.CheckVkResultFn = check_vk_result;
ImGui_ImplVulkan_Init(&init_info);



}


// // Starts a new imGui frame and sets up windows and ui elements
// void JGui::newFrame(AppContext* appContext, bool updateFrameGraph)
// {
    
//     ImGui::NewFrame();

//     // Init imGui windows and elements

//     // Debug window
//     ImGui::SetWindowPos(ImVec2(20 *  appContext->ui.scale   , 20 * appContext->ui.scale ), ImGuiCond_FirstUseEver);
//     ImGui::SetWindowSize(ImVec2(300 * appContext->ui.scale, 300 * appContext->ui.scale), ImGuiCond_Always);
//     // ImGui::TextUnformatted(example->title.c_str());
//     ImGui::TextUnformatted( device_app.getPhysicalDeviceProperties().deviceName );
    
//     //SRS - Display Vulkan API version and device driver information if available (otherwise blank)
//     // ImGui::Text("Vulkan API %i.%i.%i", VK_API_VERSION_MAJOR(device->properties.apiVersion), VK_API_VERSION_MINOR(device->properties.apiVersion), VK_API_VERSION_PATCH(device->properties.apiVersion));
//     ImGui::Text("%s %s", device_app.getDriverProperties().driverName, device_app.getDriverProperties().driverInfo);

//     // Update frame time display
//     if (updateFrameGraph) {
//         std::rotate(uiSettings.frameTimes.begin(), uiSettings.frameTimes.begin() + 1, uiSettings.frameTimes.end());
//         float frameTime = 1000.0f / (appContext->ui.frameTimer * 1000.0f);
//         uiSettings.frameTimes.back() = frameTime;
//         if (frameTime < uiSettings.frameTimeMin) {
//             uiSettings.frameTimeMin = frameTime;
//         }
//         if (frameTime > uiSettings.frameTimeMax) {
//             uiSettings.frameTimeMax = frameTime;
//         }
//     }

//     ImGui::PlotLines("Frame Times", &uiSettings.frameTimes[0], 50, 0, "", uiSettings.frameTimeMin, uiSettings.frameTimeMax, ImVec2(0, 80));

//     // ImGui::Text("Camera");
//     // ImGui::InputFloat3("position", &example->camera.position.x, 2);
//     // ImGui::InputFloat3("rotation", &example->camera.rotation.x, 2);

//     // Example settings window
//     ImGui::SetNextWindowPos(ImVec2(20 * appContext->ui.scale, 360 * appContext->ui.scale), ImGuiCond_FirstUseEver);
//     ImGui::SetNextWindowSize(ImVec2(300 * appContext->ui.scale, 200 * appContext->ui.scale), ImGuiCond_FirstUseEver);
//     ImGui::Begin("Example settings");
//     ImGui::Checkbox("Render models", &uiSettings.displayModels);
//     ImGui::Checkbox("Display logos", &uiSettings.displayLogos);
//     ImGui::Checkbox("Display background", &uiSettings.displayBackground);
//     ImGui::Checkbox("Animate light", &uiSettings.animateLight);
//     ImGui::SliderFloat("Light speed", &uiSettings.lightSpeed, 0.1f, 1.0f);
//     ImGui::ShowStyleSelector("UI style");

//     if (ImGui::Combo("UI style", &selectedStyle, "Vulkan\0Classic\0Dark\0Light\0")) {
//         setStyle(selectedStyle);
//     }

//     ImGui::End();

//     //SRS - ShowDemoWindow() sets its own initial position and size, cannot override here
//     ImGui::ShowDemoWindow();

//     // Render to generate draw buffers
//     ImGui::Render();
// }


// void JGui::updateBuffers(){
    
//     ImDrawData* imDrawData = ImGui::GetDrawData();
//     VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
//     VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

//     if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
//         return;
//     }

//     // Update buffers only if vertex or index count has been changed compared to current buffer size

//     // Vertex buffer
//     if ((vertexBuffer.buffer == VK_NULL_HANDLE) || (vertexCount != imDrawData->TotalVtxCount)) {
//         vertexBuffer.unmap();
//         vertexBuffer.destroy();
//         VK_CHECK_RESULT(device_app.createBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &vertexBuffer, vertexBufferSize));
//         vertexCount = imDrawData->TotalVtxCount;
//         vertexBuffer.map();
//     }

//     // Index buffer
//     if ((indexBuffer.buffer == VK_NULL_HANDLE) || (indexCount < imDrawData->TotalIdxCount)) {
//         indexBuffer.unmap();
//         indexBuffer.destroy();
//         VK_CHECK_RESULT(device_app.createBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &indexBuffer, indexBufferSize));
//         indexCount = imDrawData->TotalIdxCount;
//         indexBuffer.map();
//     }

//     // Upload data
//     ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer.mapped;
//     ImDrawIdx* idxDst = (ImDrawIdx*)indexBuffer.mapped;

//     for (int n = 0; n < imDrawData->CmdListsCount; n++) {
//         const ImDrawList* cmd_list = imDrawData->CmdLists[n];
//         memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
//         memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
//         vtxDst += cmd_list->VtxBuffer.Size;
//         idxDst += cmd_list->IdxBuffer.Size;
//     }

//     // Flush to make writes visible to GPU
//     vertexBuffer.flush();
//     indexBuffer.flush();

// }


// void JGui::drawFrame(VkCommandBuffer commandBuffer)
// {
//     ImGuiIO& io = ImGui::GetIO();

//     vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_obj->getPipelineLayout(), 
//                     0, 1, &descriptorSets, 0, nullptr);
//     vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

//     VkViewport viewport {};
//     viewport.width = ImGui::GetIO().DisplaySize.x;
//     viewport.height = ImGui::GetIO().DisplaySize.y;
//     viewport.minDepth = 0.0f;
//     viewport.maxDepth = 1.0f;
//     vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

//     // UI scale and translate via push constants
//     pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
//     pushConstBlock.translate = glm::vec2(-1.0f);
//     vkCmdPushConstants(commandBuffer, pipelineLayout_obj->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &pushConstBlock);

//     // Render commands
//     ImDrawData* imDrawData = ImGui::GetDrawData();
//     int32_t vertexOffset = 0;
//     int32_t indexOffset = 0;

//     if (imDrawData->CmdListsCount > 0) {

//         VkDeviceSize offsets[1] = { 0 };
//         vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
//         vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

//         for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
//         {
//             const ImDrawList* cmd_list = imDrawData->CmdLists[i];
//             for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
//             {
//                 const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
//                 VkRect2D scissorRect;
//                 scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
//                 scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
//                 scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
//                 scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
//                 vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
//                 vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
//                 indexOffset += pcmd->ElemCount;
//             }

//             vertexOffset += cmd_list->VtxBuffer.Size;

//         }
//     }
// }
