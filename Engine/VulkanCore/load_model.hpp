#pragma once
#include <vulkan/vulkan.hpp>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vector>
#include <memory>
#include <array>
#include <unordered_map>

#include <string>
#include "./global.hpp"
class JDevice;
class JBuffer;
struct JVertexBuffer;
struct JIndexBuffer;




struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    static VkVertexInputBindingDescription getBindingDescription()
    {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions()
    {
            std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};
            attributeDescriptions[0].binding = 0; // 对应的是vertex是哪一个
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);  //开头有多少个字节

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, normal);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, uv);

            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(Vertex, tangent);

            attributeDescriptions[4].binding = 0;
            attributeDescriptions[4].location = 4;
            attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[4].offset = offsetof(Vertex, bitangent);


            //binding is about vertex buffer (where there are multiple vertex buffer)
            return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && normal == other.normal && uv == other.uv && tangent == other.tangent;}

};

// namespace std {
//     template<> struct hash<Vertex> {
//         size_t operator()(Vertex const& vertex) const {
//             size_t h1 = hash<glm::vec3>()(vertex.pos);
//             size_t h2 = hash<glm::vec3>()(vertex.normal);
//             size_t h3 = hash<glm::vec2>()(vertex.uv);
//             size_t h4 = hash<glm::vec3>()(vertex.tangent);
//             return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
//         }
//     };
// }


class JModel{
  public:
    struct Builder{
        std::vector<Vertex> vertices_{}; //ensure initilaization
        std::vector<uint32_t> indices_{};

        void loadModel(const std::string& filepath);    };


    JModel(JDevice& device, const JModel::Builder& builder);
    ~JModel();
    NO_COPY(JModel);

    static std::unique_ptr<JModel> loadModelFromFile(JDevice& device, const std::string& filepath);

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

  private:
    void createVertexBuffer(const std::vector<Vertex>&vertices);
    void createIndexBuffer(const std::vector<uint32_t>&indices);

    JDevice& device_app;
    std::unique_ptr<JBuffer> vertexBuffer;
    uint32_t vertexCount;
    std::unique_ptr<JBuffer> indexBuffer;
    bool hasIndexBuffer = false;
    uint32_t indexCount;

};




