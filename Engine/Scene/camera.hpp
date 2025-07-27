#pragma once
#include <vulkan/vulkan.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "glm/gtx/euler_angles.hpp"
#include "../VulkanCore/global.hpp"


namespace Scene{


class JCameraPositioner{
public:
    virtual ~JCameraPositioner() = default;
    virtual glm::mat4 getViewMatrix() const = 0;
    virtual glm::vec3 getPosition() const = 0;
};


//--------------------------
class JCameraPositioner_firstPerson final : public JCameraPositioner{

public:
    JCameraPositioner_firstPerson() = default;
    JCameraPositioner_firstPerson ( const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up):
        cameraPosition_(pos), cameraOrientation_(glm::quat_cast(glm::lookAt(pos, target, up))), up_(up) {

        }
    
    void update(double deltaTime, const glm::vec2& mousePos, bool mousePressed);
    inline void lookAt(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up){
        cameraPosition_     = pos;
        cameraOrientation_  = glm::quat_cast(glm::lookAt(pos, target, up));
    }
    
    //getter
    virtual glm::mat4 getViewMatrix() const override;
    virtual glm::vec3 getPosition() const override        {return cameraPosition_;}
    
    //setter
    void setCameraPosition(const glm::vec3& pos)        {cameraPosition_ = pos;}
    void setSpeed(const glm::vec3& speed)               { moveSpeed_ = speed ;}
    void setUPvector(const glm::vec3& up);
    void resetMousePosition(const glm::vec3& newPos)    {mousePos_ = newPos;}
    



    ~JCameraPositioner_firstPerson(){};

private:
    glm::vec3 cameraPosition_               = glm::vec3( 0.0f, 10.0f, 10.0f);
    glm::quat cameraOrientation_            = glm::quat(glm::vec3(0));         //glm::quat(euler angles) = identityQuat (1.0f, 0.0f, 0.0f, 0.0f)

    glm::vec2 mousePos_                     = glm::vec2(0);
    glm::vec3 moveSpeed_                   = glm::vec3(0.0f);

    glm::vec3 up_                           = glm::vec3(0.0f, 0.0f, 1.0f);

public: 
    struct Movement{
        bool forward_       = false;
        bool backward_      = false;
        bool left_          = false;
        bool right_         = false;
        bool up_            = false;
        bool down_          = false;
        bool fastSpeed_     = false;
    } movement_;

    float mouseSpeed_       = 4.0f;
    float maxSpeed_         = 10.0f;

    float acceleration_     = 150.0f;
    float damping_          = 0.2f;
    float fastCoef_         = 10.0f;

};


//--------------------------
class JCamera final{

public:
    explicit JCamera(JCameraPositioner& positioner): positioner_(&positioner) {}
    NO_COPY(JCamera);

    // void setOrthoProjection(
    //     float left, float right, float top, float bottom, float near, float far);
    // void setPerspProjection(
    //     float fovy, float aspect, float near, float far);

    // void setViewDirection(
    //     glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
    // void setViewTarget(
    //     glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, -1.f, 0.f});
    // void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

    // const glm::mat4& getProjection() const      {return projectionMatrix;}
    // const glm::mat4& getView() const            { return viewMatrix; }
    // const glm::mat4& getInverseView() const     { return inverseViewMatrix; }
    // const glm::vec3  getPosition() const        { return glm::vec3(inverseViewMatrix[3]); }

    glm::mat4 getViewMatrix() const     {return positioner_->getViewMatrix();}
    glm::vec3 getPosition() const       {return positioner_->getPosition();}
    glm::mat4 getProjMatrix() const     {return proj_;}



private:
    const JCameraPositioner* positioner_;  //指向const cameraPosition的指针，通过这个指针不可修改被指向的对象
    glm::mat4 proj_;

    // glm::mat4 projectionMatrix{1.f};
    // glm::mat4 viewMatrix{1.f};
    // glm::mat4 inverseViewMatrix{1.f};










};


}