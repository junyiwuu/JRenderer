#pragma once
#include <vulkan/vulkan.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "glm/gtx/euler_angles.hpp"
#include "../VulkanCore/global.hpp"
class JWindow;
#include <glm/gtx/quaternion.hpp>



namespace Scene{


class JCameraPositioner{
public:
    virtual ~JCameraPositioner() = default;
    virtual glm::mat4 getViewMatrix() const = 0;
    virtual glm::mat4 getProjMatrix(const float ratio) const = 0;
    virtual glm::vec3 getPosition() const = 0;
};


enum class DragMode { None, Orbit, Pan, Zoom};

//--------------------------
//arcball camera implementation reference from: https://github.com/Twinklebear/arcball-cpp/tree/master

class JCameraPositioner_Arcball final : public JCameraPositioner{

public:
    JCameraPositioner_Arcball() = default;
    JCameraPositioner_Arcball (JWindow& window,const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up, const DragMode dragMode);

    // --- 核心方法 ---
    void pan  (const glm::vec2& deltaPos);
    void orbit(glm::vec2 prev_mouse, glm::vec2 cur_mouse);
    void zoom (glm::vec2 prev_mouse, glm::vec2 cur_mouse);
    
    const glm::mat4& transform() const       {return camera_;}
    const glm::mat4& invTransform() const    {return invCamera_;}

    glm::vec3 eye() const {return glm::vec3{invCamera_*glm::vec4(0, 0, 0, 1)}  ;}  //in camera world, camera always at 0
    glm::vec3 dir() const {return glm::normalize(  glm::vec3{invCamera_*glm::vec4{0, 0, -1, 0}})  ;}
    // {0, 0, -1, 0} is the camera's forward direction
    glm::vec3 up() const {return glm::normalize(   glm::vec3{invCamera_*glm::vec4{0, -1, 0, 0}})   ;}
    
    //getter
    virtual glm::mat4 getViewMatrix() const override {return camera_;}
    virtual glm::mat4 getProjMatrix(const float ratio) const override;
    virtual glm::vec3 getPosition() const override        {return  eye();}    
    
    void onMouseButton(int button, int action, double x, double y);
    void onCursorPos (double x, double y);
    void recordDragStart(double x, double y);
    
    
    ~JCameraPositioner_Arcball(){};
    

    void updateCamera();
    glm::quat rotation_;
    glm::mat4 camera_; //full camera transformation
    glm::mat4 invCamera_; // inverse camera to find camera position, world space rotation exis

    glm::mat4 center_translation_; // inverse translation that moves your chosen pivot to the world origin
    //brings the pivot point to the origin

    glm::mat4 translation_;   // how far the camera is offset from the arcball pivot


    JWindow& window_app;
    int   winWidth_, winHeight_;

    glm::vec3 eye_             ;
    glm::vec3  pivot_{0.0f};  

    glm::mat4 viewMatrix_{1.0f};
    glm::mat4 projMatrix_{1.0f};

    DragMode dragMode_        = DragMode::None;
       
    glm::vec3 up_    ;
    
    bool ifFirstPos_ = true;
    glm::vec2 preMousePos_;
    glm::vec2 currMousePos_;
    
    glm::vec2 dragStartMousePos2D_;
    glm::vec3 dragStartPos_;

    glm::quat screen_to_arcball(const glm::vec2& pos);


};


//--------------------------
class JCamera final{

public:
    explicit JCamera(JCameraPositioner& positioner): positioner_(&positioner) {}
    NO_COPY(JCamera);

    glm::mat4 getViewMatrix() const     {return positioner_->getViewMatrix();}
    glm::vec3 getPosition() const       {return positioner_->getPosition();}
    glm::mat4 getProjMatrix(const float ratio) const {return positioner_->getProjMatrix(ratio);}

private:
    const JCameraPositioner* positioner_;  //指向const cameraPosition的指针，通过这个指针不可修改被指向的对象
    glm::mat4 proj_;

};


}