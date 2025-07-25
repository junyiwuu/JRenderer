// #include "descriptorAllocator.hpp"
// #include "../device.hpp"





// JDescriptorAllocator::JDescriptorAllocator(JDevice& device):
//     device_app(device)
// {


// }


// JDescriptorAllocator::~JDescriptorAllocator(){


// }




// // descriptor pool 动态扩容简单版本：失败时自动扩容


// VkDescriptorSet JDescriptorAllocator::allocateDescriptorSet(VkDescriptorSetLayout descriptorSetLayout){
//     VkDescriptorSet descriptorSet;

//     VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
//     descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//     descriptorSetAllocInfo.descriptorPool = currentPool_;
//     descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;
//     descriptorSetAllocInfo.descriptorSetCount = 1;

//     VkResult result = vkAllocateDescriptorSets(device_app.device(), &descriptorSetAllocInfo, &descriptorSet);

//     if (result == VK_ERROR_FRAGMENTED_POOL || result == VK_ERROR_OUT_OF_POOL_MEMORY){
//         VkDescriptorPool newPool_ = 
//     }






// }


// //  set有多少是和有多少material相关的，所以适合set layout相关的？
// // 之后应该根据之前的cache来调整，现在先直接根据set layout
// VkDescriptorPool JDescriptorAllocator::createPool(){
//     VkDescriptorPoolCreateInfo descriptorPoolInfo{};
//         descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//         descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
//         descriptorPoolInfo.pPoolSizes = poolSize.data();
//         descriptorPoolInfo.maxSets = maxDescriptorSets;
//         descriptorPoolInfo.flags = poolCreateFlags;

//         if (vkCreateDescriptorPool(device_app.device(), &descriptorPoolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
//             throw std::runtime_error("failed to create descriptor pool!");}
//         }



// }




















