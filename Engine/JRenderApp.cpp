#include "JRenderApp.hpp"
#include "./VulkanCore/descriptor.hpp"

#include "./Renderers/RenderingSystem.hpp"
#include "./VulkanCore/structs/uniforms.hpp"



JRenderApp::JRenderApp(){  }


JRenderApp::~JRenderApp(){  }






void JRenderApp::run(){

    RenderingSystem renderingSystem{device_app, renderer_app.getSwapchainApp()};




    while (!glfwWindowShouldClose(window_app.getGLFWwindow())) {
        glfwPollEvents();
        

        
        // Start ImGui frame
        renderer_app.getImguiApp().newFrame();
        
        

        //if command buffer has something/working.. otherwise if it is return nullptr, will go else branch
        if(VkCommandBuffer commandBuffer = renderer_app.beginFrame()){

            auto currentFrame = renderer_app.getCurrentFrame();

            //--------- update uniform buffer------------
            static auto startTime = std::chrono::high_resolution_clock::now();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float updateUniformBuffer_time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
            // float updateUniformBuffer_time = 1;
        
            UniformBufferObject ubo{};
            ubo.model = glm::rotate(glm::mat4(1.0f), updateUniformBuffer_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f), renderer_app.getSwapchainApp().getSwapChainExtent().width / (float)renderer_app.getSwapchainApp().getSwapChainExtent().height, 0.1f, 10.0f);
            ubo.proj[1][1] *= -1;
        
            memcpy(renderingSystem.getUniformBufferObjs()[currentFrame]->getufferMapped(), 
                    &ubo, 
                    sizeof(ubo) );
            // ------------------------------------------------
        
        

            renderer_app.beginRender(commandBuffer);
            renderingSystem.render(commandBuffer, renderer_app.getCurrentFrame());

            renderer_app.getImguiApp().render(commandBuffer);

            renderer_app.endRender(commandBuffer);
            renderer_app.endFrame();



        } 
        else {
            // Frame was skipped (e.g., swapchain recreation), finish ImGui anyway
            ImGui::Render(); // Finish ImGui frame without rendering
        }
        
        // End ImGui frame
        renderer_app.getImguiApp().endFrame();


    }

    vkDeviceWaitIdle(device_app.device());

}




