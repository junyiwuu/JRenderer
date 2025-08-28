#pragma once
#include <vulkan/vulkan.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <optional>
#include <set>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>



#include "./VulkanCore/window.hpp"
#include "./VulkanCore/device.hpp"
#include "./VulkanCore/swapchain.hpp"
#include "./VulkanCore/utility.hpp"
#include "./VulkanCore/shaderModule.hpp"

#include "./VulkanCore/commandBuffer.hpp"
#include "./VulkanCore/buffer.hpp"

#include "./Interface/JImGui.hpp"
#include "./VulkanCore/sync.hpp"
#include "./VulkanCore/global.hpp"
#include "./Renderers/Renderer.hpp"
#include "./Interface/JImGui.hpp"


struct UniformBufferObject;
class Renderer;
class InteractiveSystem;
class RenderingSystem;








class JRenderApp {
public:

    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    JRenderApp();
    ~JRenderApp();
    
    NO_COPY(JRenderApp);

    void run() ;



private:
  //window
  JWindow window_app{WIDTH, HEIGHT, "vulkan"};
  JDevice device_app{window_app};
  Renderer renderer_app{window_app, device_app};
  
  std::unique_ptr<InteractiveSystem> interactiveSystem_;
  std::unique_ptr<RenderingSystem> renderingSystem_;
  
  AppContext appContext_;

  bool framebufferResized = false;

  void init();

  bool isFrameStarted{false};


  // Scene::JAsset::Map sceneAssets;



};

