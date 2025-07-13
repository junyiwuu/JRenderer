#pragma once
#include <vulkan/vulkan.h>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <unordered_map>
#include <glm/gtx/hash.hpp>
#include <string>




struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
            attributeDescriptions[0].binding = 0; // 对应的是vertex是哪一个
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);  //开头有多少个字节

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            //binding is about vertex buffer (where there are multiple vertex buffer)
            return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;}



};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}


class JModel{


public:



    JModel(const std::string& model_path);
    ~JModel();

    JModel(const JModel &) = delete;
    JModel &operator=(const JModel &) = delete;

    std::vector<Vertex> vertices() {return vertices_;}
    std::vector<uint32_t> indices() {return indices_;}


private:

    std::vector<Vertex> vertices_;
    std::vector<uint32_t> indices_;

};




