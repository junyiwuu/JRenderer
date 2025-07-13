#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <cstring>
#include <chrono>

#include "utility.hpp"
#include "load_model.hpp"
#include "device.hpp"



struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class JBuffer{

public:

    JBuffer(JDevice& device,  VkDeviceSize size, 
        VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    ~JBuffer();

    VkBuffer buffer() {return buffer_;}
    VkDeviceMemory bufferMemory() {return bufferMemory_;}
    VkDeviceSize getSize() {return size_;}

    struct externalCreateBufferResult {
        VkBuffer r_buffer_; 
        VkDeviceMemory r_bufferMemory_; };
    static externalCreateBufferResult createBuffer(JDevice& device_app,  VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void destroyBuffer(JDevice& device_app, VkBuffer buffer, VkDeviceMemory bufferMemory);



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
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT )

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


struct JIndexBuffer{
    JBuffer baseBuffer;

    JIndexBuffer(JDevice& device_app, 
                 const std::vector<uint32_t>& indices, VkCommandPool commandPool, VkQueue queue):
        baseBuffer(device_app, sizeof(indices[0])*indices.size(),
                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT )
    {
        JBuffer stagingBuffer(device_app, baseBuffer.getSize(), 
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        void* data;
        vkMapMemory(device_app.device(), stagingBuffer.bufferMemory(), 0, stagingBuffer.getSize(), 0, &data);
        memcpy(data, indices.data(), (size_t)(stagingBuffer.getSize()));
        vkUnmapMemory(device_app.device(), stagingBuffer.bufferMemory());

        util::copyBuffer(stagingBuffer.buffer(), baseBuffer.buffer(), baseBuffer.getSize(), device_app.device(), commandPool, queue);
    }
};








class JUniformBuffer{

public:

    JUniformBuffer(JDevice& device);
    ~JUniformBuffer();

    JUniformBuffer(const JUniformBuffer&) = delete;
    JUniformBuffer& operator=(const JUniformBuffer&) = delete;


    const VkBuffer& buffer() {return uniformBuffer_;}
    const VkDeviceMemory& bufferMemory(){ return uniformBufferMemory_;}
    void* bufferMapped() {return uniformBufferMapped_;}

    // void update(uint32_t currentImage, const UniformBufferObject& UniformBufferObject);

private:
    JDevice& device_app;


    VkBuffer uniformBuffer_;
    VkDeviceMemory uniformBufferMemory_;
    void* uniformBufferMapped_;




};




