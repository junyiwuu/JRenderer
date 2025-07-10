#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <cstring>

#include "utility.hpp"
#include "load_model.hpp"
#include "device.hpp"



class JBuffer{



public:

    JBuffer(JDevice& device,  VkDeviceSize size, 
        VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~JBuffer();


    VkBuffer buffer() {return buffer_;}
    VkDeviceMemory bufferMemory() {return bufferMemory_;}
    VkDeviceSize getSize() {return size_;}



private:
    JDevice& device_app;
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory bufferMemory_ = VK_NULL_HANDLE;
    VkDeviceSize size_;


};


// for vertex buffer

struct JVertexBuffer{
    JBuffer baseBuffer;
        
    JVertexBuffer(JDevice& device_app, 
                    const std::vector<Vertex>& vertices, VkCommandPool commandPool, VkQueue queue):
    //initiate JBuffer                 
        baseBuffer(device_app ,sizeof(Vertex)*vertices.size(), 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT )

        {
            JBuffer stagingBuffer(device_app, baseBuffer.getSize(), 
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            void* data;
            vkMapMemory(device_app.device(), stagingBuffer.bufferMemory(), 0, stagingBuffer.getSize(), 0, &data);
            memcpy(data, vertices.data(), (size_t)(stagingBuffer.getSize()));
            vkUnmapMemory(device_app.device(), stagingBuffer.bufferMemory());

            util::copyBuffer(stagingBuffer.buffer(), baseBuffer.buffer(), baseBuffer.getSize(), device_app.device(), commandPool, queue);

            
        }















};





















