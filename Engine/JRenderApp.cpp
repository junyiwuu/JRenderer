#include "JRenderApp.hpp"
#include "./VulkanCore/descriptor.hpp"

#include "./Renderers/RenderingSystem.hpp"







void JRenderApp::run(){

    RenderingSystem renderingSystem{device_app, renderer_app.getSwapchainApp()};




    while (!glfwWindowShouldClose(window_app.getGLFWwindow())) {
        glfwPollEvents();
        




        // Start ImGui frame
        imgui_obj->newFrame();
        





        
        bool frameRendered = drawFrame();
        
        // Always finish ImGui frame, even if rendering was skipped
        if (frameRendered) {
            // Normal frame completion handled in drawFrame
        } else {
            // Frame was skipped (e.g., swapchain recreation), finish ImGui anyway
            ImGui::Render(); // Finish ImGui frame without rendering
        }
        
        // End ImGui frame
        imgui_obj->endFrame();
    }

    vkDeviceWaitIdle(device_app.device());






    VkCommandBuffer commandBuffer = renderer_app.beginFrame();

        //--------- update uniform buffer------------
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float updateUniformBuffer_time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        // float updateUniformBuffer_time = 1;
    
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), updateUniformBuffer_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapchain_app->getSwapChainExtent().width / (float)swapchain_app->getSwapChainExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
    
        memcpy(uniformBuffer_objs[currentFrame]->getufferMapped(), &ubo, sizeof(ubo) );
        // ------------------------------------------------
    
    
    



    renderer_app.beginRender(commandBuffer);

    imgui_obj->render(commandBuffer);
    renderingSystem.render();

    renderer_app.endRender(commandBuffer);
    renderer_app.endFrame();





}




void Renderer::mainLoop() {

