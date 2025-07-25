// #pragma once


// // not done, still building



// class JTextureManager{




// public:

//     static JTextureManager loadTexture(){
//         static
//     }










// private:













// };


// class TextureManager {
//     public:
//       std::shared_ptr<Texture> load(const std::string& path) {
//         if (auto it = cache.find(path); it != cache.end())
//           return it->second;
//         auto tex = std::make_shared<Texture>(device, path);
//         cache[path] = tex;
//         return tex;
//       }
//     private:
//       Device& device;
//       std::unordered_map<std::string, std::shared_ptr<Texture>> cache;
//     };



// // Texture.hpp
// #pragma once
// #include <vulkan/vulkan.h>
// #include <string>

// class DescriptorAllocator;  // 前向声明

// class Texture {
// public:
//     Texture(VkDevice device,
//             DescriptorAllocator& alloc,
//             VkDescriptorSetLayout layout,
//             const std::string& path)
//         : _device(device)
//     {
//         // 1) 从文件加载、创建 VkImage + VkImageView + VkSampler
//         loadImageFromFile(path);
//         createImageView();
//         createSampler();

//         // 2) 分配并更新 DescriptorSet
//         _descriptorSet = alloc.allocate(layout);
//         VkDescriptorImageInfo info{
//             _sampler,
//             _imageView,
//             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
//         };
//         VkWriteDescriptorSet write{};
//         write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         write.dstSet          = _descriptorSet;
//         write.dstBinding      = 0;
//         write.dstArrayElement = 0;
//         write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//         write.descriptorCount = 1;
//         write.pImageInfo      = &info;
//         vkUpdateDescriptorSets(_device, 1, &write, 0, nullptr);
//     }

//     ~Texture() {
//         vkDestroySampler(_device,  _sampler,    nullptr);
//         vkDestroyImageView(_device, _imageView, nullptr);
//         vkDestroyImage(_device,     _image,     nullptr);
//         vkFreeMemory(_device,       _memory,    nullptr);
//         // DescriptorSet 不需要单独销毁，由 DescriptorAllocator 统一 reset
//     }

//     // 给渲染代码用
//     VkDescriptorSet descriptorSet() const { return _descriptorSet; }

//     // 给 ImGui::Image 用
//     ImTextureID getImGuiID() const {
//         return reinterpret_cast<ImTextureID>(_descriptorSet);
//     }

//     int width()  const { return _width; }
//     int height() const { return _height; }

// private:
//     void loadImageFromFile(const std::string& path);
//     void createImageView();
//     void createSampler();

//     VkDevice        _device;
//     VkImage         _image;
//     VkDeviceMemory  _memory;
//     VkImageView     _imageView;
//     VkSampler       _sampler;
//     VkDescriptorSet _descriptorSet;
//     int             _width, _height;
// };
