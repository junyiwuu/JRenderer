#include "JRenderApp.hpp"
#include "./VulkanCore/descriptor/descriptor.hpp"

#include "./Renderers/RenderingSystem.hpp"
#include "./VulkanCore/structs/uniforms.hpp"
#include "./Scene/camera.hpp"

#include "./Interface/keyboardController.hpp"

static MouseState mouseState;


JRenderApp::JRenderApp()
{ 
    MouseState mouseState{};



}


JRenderApp::~JRenderApp(){  }






void JRenderApp::run(){
    //initiate resources
    RenderingSystem renderingSystem{device_app, renderer_app.getSwapchainApp()};

        //initiate camera positioner
    const glm::vec3 initialCamPos       = glm::vec3(0.0f, 0.0f, -2.0f);
    const glm::vec3 initialCamTarget    = glm::vec3(0.0f, 0.0f, 0.0f);
    Scene::JCameraPositioner_Arcball cameraPositioner(
            window_app,
            initialCamPos, 
            initialCamTarget, 
            glm::vec3(0.0f, 1.0f, 0.0f),
            Scene::DragMode::None);
    //initiate camera
    Scene::JCamera camera(cameraPositioner);

    // Store camera positioner pointer for callbacks，存进去之后在callback的lambda可以用
    glfwSetWindowUserPointer(window_app.getGLFWwindow(), &cameraPositioner);

    // Mouse button callback - prioritize ImGui
    glfwSetMouseButtonCallback(window_app.getGLFWwindow(), 
    [](GLFWwindow* window, int button, int action, int mods){
        auto* camera = static_cast<Scene::JCameraPositioner_Arcball*>(glfwGetWindowUserPointer(window));

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        // Handle ImGui input first
        const ImGuiMouseButton_ imguiButton = (button == GLFW_MOUSE_BUTTON_LEFT)
                ? ImGuiMouseButton_Left
                : (button == GLFW_MOUSE_BUTTON_RIGHT ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);

        ImGuiIO& io = ImGui::GetIO();
        // io.MousePos = ImVec2((float)xpos, (float)ypos);
        io.MouseDown[imguiButton] = action == GLFW_PRESS;

        // Only handle camera input if ImGui doesn't want the mouse
        // if (!io.WantCaptureMouse) {
            camera->onMouseButton(button, action, xpos, ypos);
        // }
    });

    glfwSetCursorPosCallback(window_app.getGLFWwindow(), [](GLFWwindow* window, double x, double y) {
        auto* camera = static_cast<Scene::JCameraPositioner_Arcball*>(glfwGetWindowUserPointer(window));
        
        // Update ImGui mouse position
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos = ImVec2((float)x, (float)y);
        
        // Only handle camera input if ImGui doesn't want the mouse
        if (!io.WantCaptureMouse) {
            camera->onCursorPos(x, y);
        }
    });





    //set up time
    double currentTime = glfwGetTime();
    float deltaTime = 0.0f;


    while (!glfwWindowShouldClose(window_app.getGLFWwindow())) {
        glfwPollEvents(); // 遍历所有的callback
        
        const float ratio = renderer_app.getSwapchainImageAspectRatio();
        
        // get the new delta time
        double newTime  = glfwGetTime();
        deltaTime       = static_cast<float>(newTime - currentTime);
        currentTime     = newTime;        

        
        // build projection and view matrices
        const glm::mat4 perspMatrix = camera.getProjMatrix(ratio);
        const glm::mat4 viewMatrix = camera.getViewMatrix();


        // Start ImGui frame
        renderer_app.getImguiApp().newFrame();
        
        

        //if command buffer has something/working.. otherwise if it is return nullptr, will go else branch
        if(VkCommandBuffer commandBuffer = renderer_app.beginFrame()){

            auto currentFrame = renderer_app.getCurrentFrame();

            //--------- update uniform buffer------------


            GlobalUbo ubo{};
            ubo.projection = perspMatrix;
            ubo.view = viewMatrix;
            // ubo.inverseView = camera.getInverseView();
            ubo.projection[1][1] *= -1;
            
            // Debug camera position
            static int frameCount = 0;
            if (frameCount++ % 60 == 0) { // Print every 60 frames
                glm::vec3 pos = camera.getPosition();
                printf("Camera pos: (%.2f, %.2f, %.2f)\n", pos.x, pos.y, pos.z);
            }


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



