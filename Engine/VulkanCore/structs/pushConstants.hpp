#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>


struct pushConstantStruct{
    glm::vec3 offset;
    alignas(16) glm::vec3 color;
};


