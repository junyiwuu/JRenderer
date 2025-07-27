
#pragma once
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>



struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};



struct GlobalUbo {
    alignas(16) glm::mat4 projection{1.f};
    alignas(16) glm::mat4 view{1.f};
    alignas(16) glm::mat4 inverseView{1.f};

  };