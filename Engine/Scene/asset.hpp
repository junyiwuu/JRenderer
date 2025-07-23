#pragma once

#include <memory>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>

class JModel;

namespace Scene{



struct TransformComponent {
    glm::vec3 translation{};
    glm::vec3 scale{1.f, 1.f, 1.f};
    glm::vec3 rotation{};
  
    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    glm::mat4 mat4();
  
    glm::mat3 normalMatrix();
  };


class JAsset{
public:

    using id_t = unsigned int;
    using Map = std::unordered_map<id_t, JAsset>;

    static JAsset createAsset() {
        static id_t currentId = 0;
        return JAsset{currentId++};
      }


  id_t getId() { return id; }

  glm::vec3 color{};
  TransformComponent transform{};

  // Optional pointer components
  std::shared_ptr<JModel> model{};
  

private:


    JAsset(id_t objId) : id{objId} {}

    id_t id;





};



};


