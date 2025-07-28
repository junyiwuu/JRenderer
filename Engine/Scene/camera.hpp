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
class JCameraPositioner_firstPerson final : public JCameraPositioner{

public:
    JCameraPositioner_firstPerson() = default;
    JCameraPositioner_firstPerson (JWindow& window,const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up, const DragMode dragMode);


    //getter
    virtual glm::mat4 getViewMatrix() const override {return viewMatrix_;}
    virtual glm::mat4 getProjMatrix(const float ratio) const override;
    virtual glm::vec3 getPosition() const override        {return cameraPosition_;}

    void onMouseButton(int button, int action, double x, double y);
    void onCursorPos (double x, double y);
    void recordDragStart(double x, double y);

    inline void lookAt(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up){
        cameraPosition_     = pos;
        // cameraOrientation_  = glm::quat_cast(glm::lookAt(pos, target, up));
    }
    
    //setter
    void setCameraPosition(const glm::vec3& pos)        {cameraPosition_ = pos;}
    void setSpeed(const glm::vec3& speed)               { moveSpeed_ = speed ;}
    void setUPvector(const glm::vec3& up);
    void resetMousePosition(const glm::vec3& newPos)    {mousePos_ = newPos;}
    


    ~JCameraPositioner_firstPerson(){};

private:
    JWindow& window_app;
    glm::vec3 cameraPosition_               = glm::vec3( 0.0f, 0.0f, 0.0f);
    glm::vec3  pivot_{0.0f};  
    glm::vec3 upVector;
    //xyzw -> (0, 0, 0, 1)  quaternion

    glm::mat4 viewMatrix_{1.0f};
    glm::mat4 projMatrix_{1.0f};

    bool orbitDragging_ = false;
    glm::vec2 mousePos_                     = glm::vec2(0);
    glm::vec3 moveSpeed_                   = glm::vec3(0.0f);

    glm::vec2 dragStartMousePos2D_;
    glm::vec3 dragStartPos_;
    glm::quat dragStartOrient_;
    glm::vec3 dragStartSpherePos_;

    float     zoomStartDist_;  

    
    float focalDistance_ = 2.0f;

    glm::vec3 up_                           = glm::vec3(0.0f, 0.0f, 1.0f);

    // --- 参数 ---
    int   winWidth_, winHeight_;
    float      panSpeed_   = 0.001f;
    float      yawSpeed_   = 0.003f;
    float      pitchSpeed_ = 0.003f;
    float      zoomSpeed_  = 0.01f;

    DragMode dragMode_        = DragMode::None;

    // --- 核心方法 ---
    // void pan  (const glm::vec2& d);
    void orbit(const glm::vec2& d);
    // void zoom (const glm::vec2& d);
    void updateViewMatrix();
    glm::vec3  getViewDirection();
    glm::vec3 getRightVector();
    
    // Arcball helper methods
    glm::vec3 projectToSphere(float x, float y) const;
    glm::quat rotationBetweenVectors(const glm::vec3& start, const glm::vec3& dest) const;

    // 射线生成 + 射线-平面求交
    void    updateRayFromMouse(double x, double y, glm::vec3& outOrig, glm::vec3& outDir) const;
    bool    intersectRayPlane(const glm::vec3& orig,
                              const glm::vec3& dir,
                              const glm::vec3& planePoint,
                              const glm::vec3& planeNormal,
                              glm::vec3& outPoint) const;
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