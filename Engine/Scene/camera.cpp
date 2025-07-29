#include "camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>
#include "glm/gtx/euler_angles.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../VulkanCore/window.hpp"
template<typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

namespace Scene{
JCameraPositioner_Arcball::JCameraPositioner_Arcball (
    JWindow& window,
    const glm::vec3& eye, 
    const glm::vec3& pivot, 
    const glm::vec3& up, 
    const DragMode dragMode)
        :
        window_app(window),
        eye_(eye), 
        pivot_(pivot),
        up_(up) 
{
    glfwGetFramebufferSize(window_app.getGLFWwindow(), &winWidth_ , &winHeight_);

    const glm::vec3 dir = pivot_ - eye;
    glm::vec3 z_axis = glm::normalize(dir);
    glm::vec3 x_axis = glm::normalize(  glm::cross(z_axis, glm::normalize(up_)));
    glm::vec3 y_axis = glm::normalize(  glm::cross(x_axis, z_axis));

    x_axis = glm::normalize(  glm::cross(z_axis, y_axis));

    center_translation_ = glm::inverse(glm::translate(pivot_));
    translation_ = glm::translate(  glm::vec3(0.f, 0.f, -glm::length(dir))  );
    rotation_    = glm::normalize(      
                        glm::quat_cast(  
                            glm::transpose(  
                                glm::mat3(x_axis, y_axis, -z_axis))));
    updateCamera();
    projMatrix_ = getProjMatrix(winWidth_/winHeight_);
}



glm::mat4 JCameraPositioner_Arcball::getProjMatrix(const float ratio) const {
    return glm::perspective(glm::radians(60.f), ratio, 0.1f, 1000.0f);

}


void JCameraPositioner_Arcball::updateCamera(){
    camera_ = translation_ * glm::mat4_cast(rotation_) * center_translation_;
    invCamera_ = glm::inverse(camera_);
}




void JCameraPositioner_Arcball::pan(const glm::vec2& deltaPos) {
    // glm::mat4 inv_proj = glm::inverse(projMatrix_);

    glm::vec4 dxy4 = glm::inverse(projMatrix_) * glm::vec4(deltaPos.x, deltaPos.y, 0, 1);
    const float zoom_amount = std::abs(translation_[3][2]);
    glm::vec4 motion(deltaPos.x*zoom_amount , deltaPos.y*zoom_amount, 0.f, 0.f);
    motion = invCamera_ * motion;

    center_translation_ = glm::translate(glm::vec3(motion)) * center_translation_;
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

// 缩放
void JCameraPositioner_Arcball::zoom(glm::vec2 prev_mouse, glm::vec2 cur_mouse) {
    glm::vec2 deltaPos = cur_mouse - prev_mouse;
    float zoom_amount = (deltaPos.y + deltaPos.x) * 0.5f;
     
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

void JCameraPositioner_Arcball::onCursorPos(double x, double y) {
    if (dragMode_ == DragMode::None) return;

    // normalized mouse position
    currMousePos_ = glm::vec2(
        2.f * x / winWidth_ -1.f  ,  1.f - 2.f * y / winHeight_
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



glm::quat JCameraPositioner_Arcball::screen_to_arcball(const glm::vec2& mousePos){
    const float dist = glm::dot(mousePos, mousePos);

    if(dist <= 1.f){
        return glm::quat(0.0, mousePos.x, mousePos.y, std::sqrt(1.f - dist));
    }else{
        const glm::vec2 proj = glm::normalize(mousePos);
        return glm::quat(0.f, proj.x, proj.y, 0.f);
    }
}


























}