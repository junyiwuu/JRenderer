#include "load_model.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "utility.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


JModel::JModel(JDevice& device, const JModel::Builder& builder):
    device_app(device)
{
    createVertexBuffer(builder.vertices_);
    createIndexBuffer(builder.indices_);

}


JModel::~JModel(){


}

void JModel::createVertexBuffer(const std::vector<Vertex>&vertices){
    vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount>=3 && "Vertex count must be more than 3 vertices ");
    vertexBuffer = std::make_unique<JBuffer>(
            device_app ,
            sizeof(Vertex)*vertices.size(), 
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    JBuffer stagingBuffer(device_app, vertexBuffer->getSize(),   // in gpu but cpu can access
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.stagingAction(vertices.data());

    util::copyBuffer(stagingBuffer.buffer(), vertexBuffer->buffer(), vertexBuffer->getSize(), 
            device_app.device(), device_app.getCommandPool(), device_app.graphicsQueue());   

}


void JModel::createIndexBuffer(const std::vector<uint32_t>&indices){
    indexCount = static_cast<uint32_t>(indices.size());
    hasIndexBuffer = indexCount>0 ;
    if(!hasIndexBuffer){ return ;}  // if no index buffer, dont continue build

    indexBuffer = std::make_unique<JBuffer>(
                device_app, 
                sizeof(indices[0])*indices.size(),
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

    JBuffer stagingBuffer(device_app, indexBuffer->getSize(),   // in gpu but cpu can access
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    stagingBuffer.stagingAction(indices.data());

    util::copyBuffer(stagingBuffer.buffer(), indexBuffer->buffer(), indexBuffer->getSize(), 
            device_app.device(), device_app.getCommandPool(), device_app.graphicsQueue());



}

std::unique_ptr<JModel> JModel::loadModelFromFile(JDevice& device, const std::string& filepath){
    Builder builder{};
    builder.loadModel(filepath);
    return std::make_unique<JModel>(device, builder);
}


void JModel::bind(VkCommandBuffer commandBuffer){
    VkBuffer vBuffers[] = {vertexBuffer->buffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vBuffers, offsets);
    
    if(hasIndexBuffer){
        vkCmdBindIndexBuffer(commandBuffer,indexBuffer->buffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

void JModel::draw(VkCommandBuffer commandBuffer){
    if(hasIndexBuffer){
        vkCmdDrawIndexed(commandBuffer, indexCount , 1, 0, 0, 0);
    }else{
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }
}


void JModel::Builder::loadModel(const std::string& filepath){

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices_.size());
                vertices_.push_back(vertex);
            }

            indices_.push_back(uniqueVertices[vertex]);
        }
    }



}






