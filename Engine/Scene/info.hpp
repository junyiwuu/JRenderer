
#pragma once
#include "asset.hpp"




struct SceneInfo {

    // LveCamera &camera;
    Scene::JAsset::Map &assets;
  };



struct MouseState{
  glm::vec2 pos = glm::vec2(0.0f);
  bool pressedLeft = false;
};