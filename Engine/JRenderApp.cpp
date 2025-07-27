#include "JRenderApp.hpp"
#include "./VulkanCore/descriptor/descriptor.hpp"

#include "./Renderers/RenderingSystem.hpp"
#include "./VulkanCore/structs/uniforms.hpp"
#include "./Scene/camera.hpp"

#include "./Interface/keyboardController.hpp"

static MouseState mouseState;
//initiate camera positioner
const glm::vec3 initialCamPos       = glm::vec3(0.0f, 2.f, -1.f);
const glm::vec3 initialCamTarget    = glm::vec3(0.0f, 0.0f, 0.0f);
static Scene::JCameraPositioner_firstPerson cameraPositioner(initialCamPos, initialCamTarget, glm::vec3(0.0f, 1.0f, 0.0f));


JRenderApp::JRenderApp(){ 
    MouseState mouseState{};
 }


JRenderApp::~JRenderApp(){  }






void JRenderApp::run(){

    //initiate resources
    RenderingSystem renderingSystem{device_app, renderer_app.getSwapchainApp()};

    
    
    //starting viewpoint set up 
    // auto viewPoint = Scene::JAsset::createAsset();
    // viewPoint.transform.translation.z = -2.5f;

    //initiate camera
    Scene::JCamera camera(cameraPositioner);
    


    //initiate keyboard controller
    KeyboardController cameraController{};

    // mouse detection
    //      cursor detection
    glfwSetCursorPosCallback(window_app.getGLFWwindow(),    //只有当鼠标移动，才会执行lambda注册的callback，就是mousestae那两行
            [](GLFWwindow* window, double x, double y){
                int width, height;
                glfwGetFramebufferSize(window, &width, &height);
                mouseState.pos.x = static_cast<float>(x/width);
                mouseState.pos.y = 1.f - static_cast<float>(y/height);

                // ImGuiIO& io                 = ImGui::GetIO();
                // io.MousePos                 = ImVec2(  (float)x , (float)y   );
            });
    //      button detection
    glfwSetMouseButtonCallback(window_app.getGLFWwindow(),
            [](GLFWwindow* window, int button, int action, int mods){
                if(button == GLFW_MOUSE_BUTTON_LEFT){
                    mouseState.pressedLeft = action == GLFW_PRESS; }
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                // ImGuiMouseButton_ imguiButton ;
                // if(button==GLFW_MOUSE_BUTTON_LEFT){
                //     imguiButton = ImGuiMouseButton_Left;
                // }else if(button==GLFW_MOUSE_BUTTON_RIGHT){
                //     imguiButton = ImGuiMouseButton_Right;
                // }else{
                //     imguiButton = ImGuiMouseButton_Middle;
                // }

                const ImGuiMouseButton_ imguiButton = (button == GLFW_MOUSE_BUTTON_LEFT)
                                                ? ImGuiMouseButton_Left
                                                : (button == GLFW_MOUSE_BUTTON_RIGHT ? ImGuiMouseButton_Right : ImGuiMouseButton_Middle);



                ImGuiIO& io                 = ImGui::GetIO();
                io.MousePos                 = ImVec2(  (float)xpos , (float)ypos   );
                io.MouseDown[imguiButton]   = action == GLFW_PRESS;
            });

    glfwSetKeyCallback(window_app.getGLFWwindow(), [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        const bool pressed = action != GLFW_RELEASE;
        if (key == GLFW_KEY_ESCAPE && pressed)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (key == GLFW_KEY_W)
            cameraPositioner.movement_.forward_ = pressed;
        if (key == GLFW_KEY_S)
            cameraPositioner.movement_.backward_ = pressed;
        if (key == GLFW_KEY_A)
            cameraPositioner.movement_.left_ = pressed;
        if (key == GLFW_KEY_D)
            cameraPositioner.movement_.right_ = pressed;
        if (key == GLFW_KEY_1)
            cameraPositioner.movement_.up_ = pressed;
        if (key == GLFW_KEY_2)
            cameraPositioner.movement_.down_ = pressed;
        if (mods & GLFW_MOD_SHIFT)
            cameraPositioner.movement_.fastSpeed_ = pressed;
        if (key == GLFW_KEY_SPACE) {
            cameraPositioner.lookAt(initialCamPos, initialCamTarget, glm::vec3(0.0f, 1.0f, 0.0f));
            cameraPositioner.setSpeed(glm::vec3(0)); }
        });

    //set up time
    double currentTime = glfwGetTime();
    float deltaTime = 0.0f;


    while (!glfwWindowShouldClose(window_app.getGLFWwindow())) {
        glfwPollEvents();
        int width, height = 0; 
        glfwGetFramebufferSize(window_app.getGLFWwindow(), &width, &height);
        const float ratio = width / (float)height;
        
        cameraPositioner.update(deltaTime, mouseState.pos, mouseState.pressedLeft);

        // get the new delta time
        double newTime  = glfwGetTime();
        deltaTime       = static_cast<float>(newTime - currentTime);
        currentTime     = newTime;        
        //get camera's world position
        const glm::vec4 cameraPos = glm::vec4(camera.getPosition(), 1.0f);
        // build projection matrix
        const glm::mat4 perspMatrix = glm::perspective(glm::radians(60.f), ratio, 0.1f, 1000.0f);
        const glm::mat4 viewMatrix = camera.getViewMatrix();

       //  renderer_app.getSwapchainImageAspectRatio()




        // Start ImGui frame
        renderer_app.getImguiApp().newFrame();
        
        

        //if command buffer has something/working.. otherwise if it is return nullptr, will go else branch
        if(VkCommandBuffer commandBuffer = renderer_app.beginFrame()){



            auto currentFrame = renderer_app.getCurrentFrame();

            //--------- update uniform buffer------------


            GlobalUbo ubo{};
            ubo.projection = perspMatrix;
            ubo.view = camera.getViewMatrix();
            // ubo.inverseView = camera.getInverseView();
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



