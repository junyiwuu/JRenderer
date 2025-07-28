#include "camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "glm/gtx/euler_angles.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../VulkanCore/window.hpp"
template<typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

namespace Scene{
JCameraPositioner_firstPerson::JCameraPositioner_firstPerson (
    JWindow& window,
    const glm::vec3& pos, 
    const glm::vec3& target, 
    const glm::vec3& up, 
    const DragMode dragMode)
        :
        window_app(window),
        cameraPosition_(pos), 
        pivot_(target),
        up_(up) 
{
    glfwGetFramebufferSize(window_app.getGLFWwindow(), &winWidth_ , &winHeight_);
    updateViewMatrix();
    projMatrix_ = getProjMatrix(winWidth_/winHeight_);
}

void JCameraPositioner_firstPerson::setUPvector(const glm::vec3& up){
     const glm::mat4 view       = getViewMatrix();
     const glm::vec3 direction  = -glm::vec3(view[0][2], view[1][2], view[2][2]);
    //  cameraOrientation_         = glm::quat_cast(glm::lookAt(cameraPosition_, cameraPosition_ + direction, up));
}



// glm::mat4 JCameraPositioner_firstPerson::getViewMatrix() const {
//     const glm::mat4 translation = glm::translate( glm::mat4(1.0f), -cameraPosition_);
//     const glm::mat4 rotation    =  glm::mat4_cast(cameraOrientation_);
//     return  rotation * translation ;
// }

glm::mat4 JCameraPositioner_firstPerson::getProjMatrix(const float ratio) const {
    return glm::perspectiveRH_ZO(glm::radians(60.f), ratio, 0.1f, 1000.0f);

}

void JCameraPositioner_firstPerson::updateViewMatrix() {
    viewMatrix_ =  glm::lookAt(cameraPosition_, pivot_, up_);
}

glm::vec3 JCameraPositioner_firstPerson::getViewDirection(){
    return -glm::transpose(viewMatrix_)[2];
}

glm::vec3 JCameraPositioner_firstPerson::getRightVector(){
    return -glm::transpose(viewMatrix_)[0];
}


// 平移
// void JCameraPositioner_firstPerson::pan(const glm::vec2& d) {
//     float speed = panSpeed_ * focalDistance_;  // 越远的地方需要更大的位移量

//     // transfer quaternion to mat4
//     const glm::mat4 cameraO4 = glm::mat4_cast(cameraOrientation_);
//     const glm::vec3 forward     = -glm::vec3(cameraO4[0][2], cameraO4[1][2], cameraO4[2][2]);
//     const glm::vec3 right       =  glm::vec3(cameraO4[0][0], cameraO4[1][0], cameraO4[2][0]);
//     const glm::vec3 up          = glm::cross(right, forward);

//     cameraPosition_ = dragStartPos_ - right * d.x * speed  + up * d.y * speed;
// }

// 轨道旋转
void JCameraPositioner_firstPerson::orbit(const glm::vec2& deltaPos) {
    float deltaAngleX = (2* M_PI / winWidth_);
    float deltaAngleY = ( M_PI / winHeight_);

    float xAngle = -deltaPos.x * deltaAngleX * 0.01;
    float yAngle = deltaPos.y * deltaAngleY * 0.01;

    glm::vec4 position = glm::vec4(cameraPosition_, 1.0f);
    glm::vec4 pivot    = glm::vec4(pivot_ , 1.0f);

    float cosAngle = dot(getViewDirection(), up_);
    if(cosAngle * sgn(deltaAngleY) > 0.99f){
        deltaAngleY = 0;
    }

    glm::mat4 rotationMatrixX{1.0f};
    rotationMatrixX = glm::rotate(rotationMatrixX, xAngle, up_);
    position = (rotationMatrixX * (position - pivot) )+ pivot;

    glm::mat4 rotationMatrixY{1.0f};
    rotationMatrixY = glm::rotate(rotationMatrixY, yAngle, getRightVector());
    glm::vec3 finalPosition = (rotationMatrixY * (position - pivot) )+ pivot;

    cameraPosition_ = finalPosition;
    pivot_ = pivot;
    updateViewMatrix();

}

// // 缩放
// void JCameraPositioner_firstPerson::zoom(const glm::vec2& deltaPos) {
//     float factor = 1.0f + deltaPos.y * zoomSpeed_;
//     factor = glm::clamp(factor, 0.1f, 10.0f);
//     float newDist = zoomStartDist_ * factor;

//     glm::vec3 dir = glm::normalize(dragStartPos_ - pivot_);
//     cameraPosition_ = pivot_ + dir * newDist;
//     // 保持看向 pivot
//     cameraOrientation_ = glm::quat_cast(
//         glm::lookAtRH(cameraPosition_, pivot_, glm::vec3(0,1,0))
//     );
// }





void JCameraPositioner_firstPerson::onMouseButton(int button, int action, double x, double y){
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

void JCameraPositioner_firstPerson::recordDragStart(double x, double y) {
    dragStartMousePos2D_ = glm::vec2(float(x), float(y));
    dragStartPos_        = cameraPosition_;
    // dragStartOrient_     = cameraOrientation_;

    // if (dragMode_ == DragMode::Orbit || dragMode_ == DragMode::Zoom) {

    //     glm::vec3 orig, dir;
    //     updateRayFromMouse(x, y, orig, dir);

    //     // 构造focal plane：法线=相机forward，平面点=P0
    //     glm::vec3 forward = cameraOrientation_ * glm::vec3(0,0,-1); //得到在world里面，相机的“向前看”指的是哪里
    //     // quaternion绕像连旋转，返回的是vec3， 是vec3(0,0,-1)按照quaternion camera orientation旋转后的结果
    //     std::cout << "forward direction" << glm::to_string(forward) << std::endl;

    //     glm::vec3 P0      = cameraPosition_ + forward * focalDistance_;
    //     std::cout << "P0  position" << glm::to_string(P0) << std::endl;
    //     // hit point
    //     glm::vec3 hit;
    //     if (intersectRayPlane(orig, dir, P0, forward, hit)) {
    //         // pivot_ = hit;
    //         std::cout << "here interset ray plane branch" << std::endl;
    //         std::cout << "Pivot  position" << glm::to_string(pivot_) << std::endl;
    //         if (dragMode_ == DragMode::Zoom)
    //             zoomStartDist_ = glm::length(dragStartPos_ - pivot_);
    //     }
    //     else {
    //         // fallback：如果没交到，就取焦平面中心

    //         pivot_ = P0;
    //         std::cout << "here NOT interset ray plane branch" << std::endl;
    //         std::cout << "Pivot  position" << glm::to_string(pivot_) << std::endl;
    //         zoomStartDist_ = focalDistance_;
    //     }
    // }
       
}




// 从鼠标屏幕坐标生成世界射线
void JCameraPositioner_firstPerson::updateRayFromMouse(double x, double y, glm::vec3& outOrig, glm::vec3& outDir) const {
    // scnreen space [0, 1]
    float s_x = float(x) / float(winWidth_);
    float s_y = float(y) / float(winHeight_);
    // [0,1]->[-1,1] and flip y
    float ndcX = s_x * 2.0f - 1.0f;
    float ndcY = 1.0f - s_y * 2.0f;
    // clip space
    glm::vec4 clipNear{ ndcX, ndcY,  0.0f, 1.0f };
    glm::vec4 clipFar { ndcX, ndcY,  1.0f, 1.0f };
    // inverse clip space  // 讲clipsppace的乘以invClip得到的是世界空间坐标，三维里面的真实位置
    glm::mat4 invClip = glm::inverse(projMatrix_ * viewMatrix_);
    // clip space里的两个点inverse回了世界空间坐标
    glm::vec4 worldNear = invClip * clipNear; 
    worldNear /= worldNear.w;
    glm::vec4 worldFar  = invClip * clipFar;  
    worldFar  /= worldFar .w;

    outOrig = glm::vec3(worldNear);
    outDir  = glm::normalize(glm::vec3(worldFar - worldNear));
}


bool JCameraPositioner_firstPerson::intersectRayPlane(
    const glm::vec3& orig,
    const glm::vec3& dir,
    const glm::vec3& planePoint,
    const glm::vec3& planeNormal,
    glm::vec3& outPoint) const
{
    float denom = glm::dot(dir, planeNormal);  // dir和plannormal是否一致方向，一致就是1
    if (fabs(denom) < 1e-6f) return false;
    float t = glm::dot(planePoint - orig, planeNormal) / denom;
    if (t < 0) return false;
    outPoint = orig + dir * t;
    return true;
}






void JCameraPositioner_firstPerson::onCursorPos(double x, double y) {
    if (dragMode_ == DragMode::None) return;
    glm::vec2 currentPos{float(x), float(y)};
    glm::vec2 delta = currentPos - dragStartMousePos2D_;

    switch (dragMode_) {
    //   case DragMode::Pan:   pan(delta);   break;
      case DragMode::Orbit: orbit(delta); break;
    //   case DragMode::Zoom:  zoom(delta);  break;
      default: break;
    }
    updateViewMatrix();
}




























}