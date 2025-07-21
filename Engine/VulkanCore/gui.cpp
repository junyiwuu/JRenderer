#include "gui.hpp"




JGui::JGui(JDevice& device ):
        device_app(device)  
{
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.0f;
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.0f);
}


JGui::~JGui(){
    ImGui::DestroyContext();


}

void JGui::initResources(VkRenderPass renderPass, VkQueue queue, const std::string& shadersPath)
{
    ImGuiIO& io = ImGui::GetIO();

    // create font texture
    unsigned char* fontData;
    int texWidth, texHeight;
    io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
    VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(fontData); // because of 4 channels

    auto imageInfo = ImageCreateInfoBuilder(texWidth, texHeight)
                    .format(VK_FORMAT_R8G8B8A8_UNORM)
                    .usage(VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                    .getInfo();
    VkResult res_img = device_app.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, fontImage_, fontMemory_);
    assert(res_img == VK_SUCCESS && "failed to create UI font image!");

    auto viewInfo = ImageViewCreateInfoBuilder(fontImage_)
                    .format(VK_FORMAT_R8G8B8A8_UNORM)
                    .getInfo();
    VkResult res_view = device_app.createImageViewWithInfo(viewInfo, fontImageView_);
    assert(res_view == VK_SUCCESS && "failed to create UI font image view!");

    
    JBuffer stagingBuffer(device_app, uploadSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.stagingAction(fontData);

    // copy buffer data to font image
    JCommandBuffer copyBuffer(device_app, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    copyBuffer.beginSingleTimeCommands();
    device_app.transitionImageLayout(copyBuffer.getCommandBuffer(), fontImage_, 
                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                    VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
                    VK_IMAGE_ASPECT_COLOR_BIT, 1);
    JTexture::copyBufferToImage(copyBuffer.getCommandBuffer(), stagingBuffer.buffer(), fontImage_, texWidth, texHeight);
    device_app.transitionImageLayout(copyBuffer.getCommandBuffer(), fontImage_, 
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
                    VK_IMAGE_ASPECT_COLOR_BIT, 1);
    copyBuffer.endSingleTimeCommands(device_app.graphicsQueue());

    // font texture sampler
    auto fontSamplerInfo = SamplerCreateInfoBuilder()
                            .addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
                            .borderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
                            .getInfo();
    vkCreateSampler(device_app.device(), &fontSamplerInfo, nullptr, &sampler_);

    //descriptor
    descriptorSetLayout_obj = JDescriptorSetLayout::Builder(device_app)
                            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                            .build();                        
    descriptorPool_obj = JDescriptorPool::Builder(device_app)
                        .reservePoolDescriptors(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1)
                        .setMaxSets(2)
                        .build();
    JDescriptorWriter writer(*descriptorSetLayout_obj, *descriptorPool_obj);
    VkDescriptorImageInfo des_imgInfo{};
    des_imgInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
    des_imgInfo.imageView = fontImageView_;
    des_imgInfo.sampler = sampler_;
    if(!writer.writeImage(0, &des_imgInfo).build(descriptorSets)){
        throw std::runtime_error("failed to build font image descriptor set!");}


    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK_RESULT(vkCreatePipelineCache(device_app.device(), &pipelineCacheCreateInfo, nullptr, &pipelineCache));

    // push constant for ui render

    VkPushConstantRange pushConstRange{};
    pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstRange.offset = 0;
    pushConstRange.size = sizeof(PushConstBlock);
    
    //pipeline
    PipelineConfigInfo pipelineCfg{};
    JPipeline::defaultPipelineConfigInfo(pipelineCfg);

    


    

}
