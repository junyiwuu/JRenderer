#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>


struct pushConstantStruct{
    glm::vec3 offset;
    alignas(16) glm::vec3 color;
};


struct pushTransformation{
    glm::mat4 modelMatrix{1.f};
};



struct pushBRDFStruct{
    uint32_t BRDF_W;
    uint32_t BRDF_H;
    uint64_t bufferAddr;
};

























