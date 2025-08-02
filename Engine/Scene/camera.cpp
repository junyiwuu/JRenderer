#include "camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "glm/gtx/euler_angles.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../VulkanCore/window.hpp"
#include <cmath>
#include <iostream>
template<typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

namespace Scene{
JCameraPositioner_Arcball::JCameraPositioner_Arcball (
    const glm::vec3& eye, 
    const glm::vec3& pivot, 
    const glm::vec3& up, 
    const DragMode dragMode)
        :
        eye_(eye), 
        pivot_(pivot),
        up_(up) 
{
    // Window dimensions will be fetched dynamically when needed

    const glm::vec3 dir = pivot_ - eye;
    glm::vec3 z_axis = glm::normalize(dir);
    glm::vec3 x_axis = glm::normalize(  glm::cross(z_axis, glm::normalize(up_)));
    glm::vec3 y_axis = glm::normalize(  glm::cross(x_axis, z_axis));

    x_axis = glm::normalize(  glm::cross(z_axis, y_axis));

    pivot_translation_ = glm::inverse(glm::translate(pivot_));

    translation_ = glm::translate(  glm::vec3(0.f, 0.f, -glm::length(dir))  );

    rotation_    = glm::normalize(      
                        glm::quat_cast(  
                            glm::transpose(  
                                glm::mat3(x_axis, y_axis, -z_axis))));
    updateCamera();
    // Projection matrix will be calculated dynamically in getProjMatrix() - no need to cache
}



glm::mat4 JCameraPositioner_Arcball::getProjMatrix(const float ratio) const {
    // std::cerr << "DEBUG: Arcball getProjMatrix called with ratio: " << ratio << std::endl;
    
    // Check for invalid ratio
    if (!std::isfinite(ratio) || ratio <= 0.0f) {
        // std::cerr << "ERROR: Invalid aspect ratio: " << ratio << std::endl;
        return glm::perspective(glm::radians(60.f), 1.0f, 0.1f, 1000.0f); // Fallback
    }
    
    auto result = glm::perspective(glm::radians(60.f), ratio, 0.1f, 1000.0f);
    // std::cerr << "DEBUG: Arcball getProjMatrix returning successfully" << std::endl;
    return result;
}


void JCameraPositioner_Arcball::updateCamera(){
    // move pivot to world 0 -> rotate -> move (rotated scene) along camera's new view direction, do translation
    viewMatrix_ = translation_ * glm::mat4_cast(rotation_) * pivot_translation_;
    invViewMatrix_ = glm::inverse(viewMatrix_);
}




void JCameraPositioner_Arcball::pan(const glm::vec2& deltaPos) {
    // glm::mat4 inv_proj = glm::inverse(projMatrix_);

    // glm::vec4 dxy4 = glm::inverse(projMatrix_) * glm::vec4(deltaPos.x, deltaPos.y, 0, 1);


    const float zoom_amount = std::abs(translation_[3][2]) * pan_speed_;
    glm::vec4 motion(deltaPos.x*zoom_amount , deltaPos.y*zoom_amount, 0.f, 0.f);
    motion = invViewMatrix_ * motion;

    pivot_translation_ = glm::translate(glm::vec3(motion)) * pivot_translation_;
    updateCamera();

}


void JCameraPositioner_Arcball::orbit(glm::vec2 prev_mouse, glm::vec2 cur_mouse) {
    
    cur_mouse = glm::clamp(cur_mouse, glm::vec2{-1, -1}, glm::vec2{1, 1});
    prev_mouse = glm::clamp(prev_mouse, glm::vec2{-1, -1}, glm::vec2{1, 1});

    const glm::quat mouse_curr_ball = screen_to_arcball(cur_mouse);
    const glm::quat mouse_prev_ball = screen_to_arcball(prev_mouse);

    rotation_ = mouse_curr_ball * mouse_prev_ball * rotation_;
    updateCamera();
}



glm::quat JCameraPositioner_Arcball::screen_to_arcball(const glm::vec2& mousePos){
    const float dist = glm::dot(mousePos, mousePos) * orbit_speed_;

    if(dist <= 1.f){
        return glm::quat(0.0, mousePos.x, mousePos.y, std::sqrt(1.f - dist));
    }else{
        const glm::vec2 proj = glm::normalize(mousePos);
        return glm::quat(0.f, proj.x, proj.y, 0.f);
    }
}







// 缩放
void JCameraPositioner_Arcball::zoom(glm::vec2 prev_mouse, glm::vec2 cur_mouse) {
    glm::vec2 deltaPos = cur_mouse - prev_mouse;
    float zoom_amount = (deltaPos.y + deltaPos.x) * zoom_speed_;
     
    const glm::vec3 motion(0.f , 0.f, zoom_amount);  //think in mat4 homogeneous matrix
    translation_ = glm::translate(motion) * translation_;

    updateCamera();
}


void JCameraPositioner_Arcball::onMouseButton(int button, int action, double x, double y){
    if (action == GLFW_PRESS) {
        if      (button == GLFW_MOUSE_BUTTON_LEFT)   dragMode_ = DragMode::Orbit;
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE) dragMode_ = DragMode::Pan;
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)  dragMode_ = DragMode::Zoom;
        recordDragStart(x, y);
    }
    else if (action == GLFW_RELEASE) {
        dragMode_ = DragMode::None;
    }
}

void JCameraPositioner_Arcball::recordDragStart(double x, double y) {
    dragStartMousePos2D_ = glm::vec2(float(x), float(y));
    dragStartPos_        = eye_;   
    ifFirstPos_ = true;
    // preMousePos_ = glm::vec2(float(x), float(y));
}

void JCameraPositioner_Arcball::onCursorPos(double x, double y, GLFWwindow* window) {
    if (dragMode_ == DragMode::None) return;
    int window_width, window_height;
    glfwGetFramebufferSize(window, &window_width, &window_height);
    
    // Safety check for invalid window dimensions during resize
    if (window_width <= 0 || window_height <= 0) {
        std::cerr << "WARNING: Invalid window dimensions during resize: " << window_width << "x" << window_height << std::endl;
        return; // Skip this frame
    }

    // normalized mouse position
    currMousePos_ = glm::vec2(
        2.f * x / window_width -1.f  ,  1.f - 2.f * y / window_height
    );

    if(ifFirstPos_){
        preMousePos_.x = currMousePos_.x;
        preMousePos_.y = currMousePos_.y;
        ifFirstPos_ = false;
    }
    
    glm::vec2 delta = currMousePos_ - preMousePos_;
    
    switch (dragMode_) {
      case DragMode::Pan:   pan(delta);   break;
      case DragMode::Orbit: orbit(preMousePos_, currMousePos_); break;
      case DragMode::Zoom:  zoom(preMousePos_, currMousePos_);  break;
      default: break;
    }
    updateCamera();
    preMousePos_ = currMousePos_;
}




//---------------------------------

void JCameraPositioner_firstPerson::onMouseButton(int button, int action, double x, double y){
    if (action == GLFW_PRESS) {
        if      (button == GLFW_MOUSE_BUTTON_LEFT)   inputMode_ = InputMode::MouseLeft;
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE) inputMode_ = InputMode::MouseMiddle;
        else if (button == GLFW_MOUSE_BUTTON_RIGHT)  inputMode_ = InputMode::MouseRight;
        // recordDragStart(x, y);
    }
    else if (action == GLFW_RELEASE) {
        inputMode_ = InputMode::None;
    }
}

void JCameraPositioner_firstPerson::onKeyboardButton(int key, int action){
    if (action == GLFW_PRESS) {
        if      (key == GLFW_KEY_W)     inputMode_ = InputMode::Key_W;
        else if (key == GLFW_KEY_S)     inputMode_ = InputMode::Key_S;
        else if (key == GLFW_KEY_A)     inputMode_ = InputMode::Key_A;
        else if (key == GLFW_KEY_D)     inputMode_ = InputMode::Key_D;
        else if (key == GLFW_KEY_1)     inputMode_ = InputMode::Key_1;
        else if (key == GLFW_KEY_2)     inputMode_ = InputMode::Key_2;
        
        // recordDragStart(x, y);
    }
    else if (action == GLFW_RELEASE) {
        inputMode_ = InputMode::None;
    }
}


void JCameraPositioner_firstPerson::setUPvector(const glm::vec3& up){
    const glm::mat4 view       = getViewMatrix();
    const glm::vec3 direction  = -glm::vec3(view[0][2], view[1][2], view[2][2]);  // get forward direction from view matrix
    cameraOrientation_         = (glm::lookAt(cameraPosition_, cameraPosition_ + direction, up));  // world position -> camera position
}



glm::mat4 JCameraPositioner_firstPerson::getViewMatrix() const {
   const glm::mat4 translation = glm::translate( glm::mat4(1.0f), -cameraPosition_);
   const glm::mat4 rotation    =  glm::mat4_cast(cameraOrientation_);
   return  rotation * translation ;
}



void JCameraPositioner_firstPerson::update(
       double deltaTime, 
       const glm::vec2& newMousePos, 
       bool mousePressed)
{
   if (mousePressed){
       const glm::vec2 deltaPos = newMousePos - mousePos_;
       //euler angle: glm::vec3(pitch, yaw, 0)
       const glm::quat deltaQuat = glm::quat(glm::vec3 (  
           -mouseSpeed_*deltaPos.y , mouseSpeed_*deltaPos.x , 0.0f  ));
       cameraOrientation_ = deltaQuat * cameraOrientation_;
       cameraOrientation_ = glm::normalize(cameraOrientation_);
       setUPvector(up_);
   }
   mousePos_ = newMousePos;

   // transfer quaternion to mat4
   const glm::mat4 cameraO4 = glm::mat4_cast(cameraOrientation_);
   const glm::vec3 forward     = -glm::vec3(cameraO4[0][2], cameraO4[1][2], cameraO4[2][2]);
   const glm::vec3 right       =  glm::vec3(cameraO4[0][0], cameraO4[1][0], cameraO4[2][0]);
   const glm::vec3 up          = glm::cross(right, forward);

   glm::vec3 accel(0.0f);
   if(inputMode_ == InputMode::Key_W)       accel+= forward;
   if(inputMode_ == InputMode::Key_S)       accel -= forward;
   if(inputMode_ == InputMode::Key_A)       accel -= right;
   if(inputMode_ == InputMode::Key_D)       accel += right;
   if(inputMode_ == InputMode::Key_1)       accel += up; 
   if(inputMode_ == InputMode::Key_2)       accel -= up;
   if(movement_.fastSpeed_)    accel *= fastCoef_;

   if(accel == glm::vec3(0.0f)){
       moveSpeed_ -= moveSpeed_ * std::min(  (1.0f/damping_) * static_cast<float>(deltaTime), 1.0f );
   }else{
       //acceleration
       moveSpeed_ += accel * acceleration_ * static_cast<float>(deltaTime);
       const float newMaxSpeed = movement_.fastSpeed_ ? maxSpeed_*fastCoef_ : maxSpeed_; //if fastSpped is true, choose maxspeed*fastcoef, otherwise maxspeed
       if (glm::length(moveSpeed_) > newMaxSpeed){
           moveSpeed_ = glm::normalize(moveSpeed_) * newMaxSpeed;
       }
   cameraPosition_ += moveSpeed_ * static_cast<float>(deltaTime);
   }
}


glm::mat4 JCameraPositioner_firstPerson::getProjMatrix(const float ratio) const {
    return glm::perspective(glm::radians(60.f), ratio, 0.1f, 1000.0f);

}


















}