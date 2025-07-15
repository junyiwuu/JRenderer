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
    


    

}
