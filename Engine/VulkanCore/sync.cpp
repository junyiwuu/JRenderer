#include "sync.hpp"
#include "device.hpp"

// only one group


JSync::JSync(JDevice& device):
    device_app(device)    
{

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_CHECK_RESULT(vkCreateSemaphore(device_app.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphore));
    VK_CHECK_RESULT(vkCreateSemaphore(device_app.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphore));
    VK_CHECK_RESULT(vkCreateFence(device_app.device(), &fenceInfo, nullptr, &inFlightFence));
  
}


JSync::~JSync(){
    vkDestroySemaphore(device_app.device(), imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(device_app.device(), renderFinishedSemaphore, nullptr);
    vkDestroyFence(device_app.device(), inFlightFence, nullptr);
}









