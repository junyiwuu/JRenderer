#include "camera.hpp"
#include "glm/gtx/euler_angles.hpp"


namespace Scene{



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
    if(movement_.forward_)      accel+= forward;
    if(movement_.backward_)     accel -= forward;
    if(movement_.left_)         accel -= right;
    if(movement_.right_)        accel += right;
    if(movement_.up_)           accel += up; 
    if(movement_.down_)         accel -= up;
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














    
// void JCamera::setOrthoProjection(
//     float left, float right, float top, float bottom, float near, float far) {
//     projectionMatrix = glm::mat4{1.0f};
//     projectionMatrix[0][0] = 2.f / (right - left);
//     projectionMatrix[1][1] = 2.f / (bottom - top);
//     projectionMatrix[2][2] = 1.f / (far - near);
//     projectionMatrix[3][0] = -(right + left) / (right - left);
//     projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
//     projectionMatrix[3][2] = -near / (far - near);
// }

// void JCamera::setPerspProjection(float fovy, float aspect, float near, float far) {
//     assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
//     const float tanHalfFovy = tan(fovy / 2.f);
//     projectionMatrix = glm::mat4{0.0f};
//     projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
//     projectionMatrix[1][1] = 1.f / (tanHalfFovy);
//     projectionMatrix[2][2] = far / (far - near);
//     projectionMatrix[2][3] = 1.f;
//     projectionMatrix[3][2] = -(far * near) / (far - near);
// }


// void JCamera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
//     const glm::vec3 w{glm::normalize(direction)};
//     const glm::vec3 u{glm::normalize(glm::cross(w, up))};
//     const glm::vec3 v{glm::cross(w, u)};

//     viewMatrix = glm::mat4{1.f};
//     viewMatrix[0][0] = u.x;
//     viewMatrix[1][0] = u.y;
//     viewMatrix[2][0] = u.z;
//     viewMatrix[0][1] = v.x;
//     viewMatrix[1][1] = v.y;
//     viewMatrix[2][1] = v.z;
//     viewMatrix[0][2] = w.x;
//     viewMatrix[1][2] = w.y;
//     viewMatrix[2][2] = w.z;
//     viewMatrix[3][0] = -glm::dot(u, position);
//     viewMatrix[3][1] = -glm::dot(v, position);
//     viewMatrix[3][2] = -glm::dot(w, position);

//     inverseViewMatrix = glm::mat4{1.f};
//     inverseViewMatrix[0][0] = u.x;
//     inverseViewMatrix[0][1] = u.y;
//     inverseViewMatrix[0][2] = u.z;
//     inverseViewMatrix[1][0] = v.x;
//     inverseViewMatrix[1][1] = v.y;
//     inverseViewMatrix[1][2] = v.z;
//     inverseViewMatrix[2][0] = w.x;
//     inverseViewMatrix[2][1] = w.y;
//     inverseViewMatrix[2][2] = w.z;
//     inverseViewMatrix[3][0] = position.x;
//     inverseViewMatrix[3][1] = position.y;
//     inverseViewMatrix[3][2] = position.z;
// }


// void JCamera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
//     setViewDirection(position, target - position, up);
// }

// void JCamera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
//     const float c3 = glm::cos(rotation.z);
//     const float s3 = glm::sin(rotation.z);
//     const float c2 = glm::cos(rotation.x);
//     const float s2 = glm::sin(rotation.x);
//     const float c1 = glm::cos(rotation.y);
//     const float s1 = glm::sin(rotation.y);
//     const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
//     const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
//     const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
//     viewMatrix = glm::mat4{1.f};
//     viewMatrix[0][0] = u.x;
//     viewMatrix[1][0] = u.y;
//     viewMatrix[2][0] = u.z;
//     viewMatrix[0][1] = v.x;
//     viewMatrix[1][1] = v.y;
//     viewMatrix[2][1] = v.z;
//     viewMatrix[0][2] = w.x;
//     viewMatrix[1][2] = w.y;
//     viewMatrix[2][2] = w.z;
//     viewMatrix[3][0] = -glm::dot(u, position);
//     viewMatrix[3][1] = -glm::dot(v, position);
//     viewMatrix[3][2] = -glm::dot(w, position);

//     inverseViewMatrix = glm::mat4{1.f};
//     inverseViewMatrix[0][0] = u.x;
//     inverseViewMatrix[0][1] = u.y;
//     inverseViewMatrix[0][2] = u.z;
//     inverseViewMatrix[1][0] = v.x;
//     inverseViewMatrix[1][1] = v.y;
//     inverseViewMatrix[1][2] = v.z;
//     inverseViewMatrix[2][0] = w.x;
//     inverseViewMatrix[2][1] = w.y;
//     inverseViewMatrix[2][2] = w.z;
//     inverseViewMatrix[3][0] = position.x;
//     inverseViewMatrix[3][1] = position.y;
//     inverseViewMatrix[3][2] = position.z;
// }








}