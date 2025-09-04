#include "JRenderApp.hpp"
#include "./VulkanCore/descriptor/descriptor.hpp"

#include "./Renderers/RenderingSystem.hpp"
#include "./VulkanCore/structs/uniforms.hpp"


#include "./Interface/keyboardController.hpp"
#include "./Renderers/interactiveSystem.hpp"

static MouseState mouseState;


JRenderApp::JRenderApp()
{ 
    MouseState mouseState{};

    renderingSystem_ = std::make_unique<RenderingSystem>(device_app, renderer_app.getSwapchainApp());
    interactiveSystem_ = std::make_unique<InteractiveSystem>(window_app, device_app, renderer_app.getSwapchainApp());
    
    // Setup AppContext with all components
    appContext_.setWindow(&window_app);
    appContext_.setInteractiveSystem(interactiveSystem_.get());
    appContext_.registerAllCallbacks(window_app.getGLFWwindow());

}


JRenderApp::~JRenderApp(){  }






void JRenderApp::run(){
    //initiate resources
    // RenderingSystem renderingSystem{device_app, renderer_app.getSwapchainApp()};
    




    while (!glfwWindowShouldClose(window_app.getGLFWwindow())) {
        glfwPollEvents(); // 遍历所有的callback
        
        const float ratio = renderer_app.getSwapchainImageAspectRatio();
        // std::cout << "DEBUG:: current ratio" << ratio <<std::endl;
                
        // build projection and view matrices
        const glm::mat4 perspMatrix = interactiveSystem_->getProjMatrix(ratio);;
        const glm::mat4 viewMatrix = interactiveSystem_->getViewMatrix();


        // Start ImGui frame
        interactiveSystem_->getImguiApp().newFrame();
        
        

        //if command buffer has something/working.. otherwise if it is return nullptr, will go else branch
        if(VkCommandBuffer commandBuffer = renderer_app.beginFrame()){

            auto currentFrame = renderer_app.getCurrentFrame();

            //--------- update uniform buffer------------
            GlobalUbo ubo{};
            ubo.projection = perspMatrix;
            ubo.view = viewMatrix;
            ubo.projection[1][1] *= -1;
            ubo.invView = glm::inverse(viewMatrix);
            ubo.camPos = glm::vec3(ubo.invView[3]);
            
            // Debug camera position
            // static int frameCount = 0;
            // if (frameCount++ % 60 == 0) { // Print every 60 frames
            //     glm::vec3 pos = camera.getPosition();
            //     printf("Camera pos: (%.2f, %.2f, %.2f)\n", pos.x, pos.y, pos.z);
            // }


            memcpy(renderingSystem_->getUniformBufferObjs()[currentFrame]->getBufferMapped(), 
                    &ubo, 
                    sizeof(ubo) );
            // ------------------------------------------------
            


            renderer_app.beginRender(commandBuffer);
            renderingSystem_->render(commandBuffer, renderer_app.getCurrentFrame());

            interactiveSystem_->getImguiApp().render(commandBuffer);

            renderer_app.endRender(commandBuffer);
            renderer_app.endFrame();



        } 
        else {
            // Frame was skipped (e.g., swapchain recreation), finish ImGui anyway
            ImGui::Render(); // Finish ImGui frame without rendering
        }
        
        // End ImGui frame
        interactiveSystem_->getImguiApp().endFrame();


    }

    vkDeviceWaitIdle(device_app.device());

}



