#include "JRenderApp.hpp"
#include "./VulkanCore/descriptor/descriptor.hpp"

#include "./Renderers/RenderingSystem.hpp"
#include "./VulkanCore/structs/uniforms.hpp"
#include "./Scene/camera.hpp"

#include "./Interface/keyboardController.hpp"




JRenderApp::JRenderApp(){ 
    
 }


JRenderApp::~JRenderApp(){  }






void JRenderApp::run(){

    //initiate resources
    RenderingSystem renderingSystem{device_app, renderer_app.getSwapchainApp()};
    //initiate camera
    Scene::JCamera camera{};
    
    //starting viewpoint set up 
    auto viewPoint = Scene::JAsset::createAsset();
    viewPoint.transform.translation.z = -2.5f;

    //initiate keyboard controller
    KeyboardController cameraController{};




    //set up time
    auto currentTime = std::chrono::high_resolution_clock::now();


    while (!glfwWindowShouldClose(window_app.getGLFWwindow())) {
        glfwPollEvents();
        
        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime =  // per frame time
            std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        cameraController.moveInPlaneXZ(window_app.getGLFWwindow(), frameTime, viewPoint);
        camera.setViewYXZ(viewPoint.transform.translation, viewPoint.transform.rotation);
        
        // Update projection matrix with current aspect ratio
        camera.setPerspProjection(glm::radians(50.f), renderer_app.getSwapchainImageAspectRatio(), 0.1f, 100.f);







        // Start ImGui frame
        renderer_app.getImguiApp().newFrame();
        
        

        //if command buffer has something/working.. otherwise if it is return nullptr, will go else branch
        if(VkCommandBuffer commandBuffer = renderer_app.beginFrame()){

            // // get the scene info, which including all assets
            // SceneInfo sceneInfo{
            //     sceneAssets,
            // };


            auto currentFrame = renderer_app.getCurrentFrame();

            //--------- update uniform buffer------------


            GlobalUbo ubo{};
            ubo.projection = camera.getProjection();
            ubo.view = camera.getView();
            ubo.inverseView = camera.getInverseView();
            ubo.projection[1][1] *= -1;


            memcpy(renderingSystem.getUniformBufferObjs()[currentFrame]->getBufferMapped(), 
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



