#pragma once
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
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
enum class InputMode { None, MouseLeft, MouseRight, MouseMiddle, 
  Key_W, Key_A, Key_S, Key_D,   Key_1, Key_2};

  
//--------------------------
//arcball camera implementation reference from: https://github.com/Twinklebear/arcball-cpp/tree/master


class JCameraPositioner_Arcball final : public JCameraPositioner{

public:
    JCameraPositioner_Arcball() = default;  
    JCameraPositioner_Arcball (const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up, const DragMode dragMode);

    // --- 核心方法 ---
    void pan  (const glm::vec2& deltaPos);
    void orbit(glm::vec2 prev_mouse, glm::vec2 cur_mouse);
    void zoom (glm::vec2 prev_mouse, glm::vec2 cur_mouse);
    
    const glm::mat4& transform() const       {return viewMatrix_;}
    const glm::mat4& invTransform() const    {return invViewMatrix_;}

    glm::vec3 eye() const {return glm::vec3{invViewMatrix_*glm::vec4(0, 0, 0, 1)}  ;}  //in camera world, camera always at 0
    glm::vec3 dir() const {return glm::normalize(  glm::vec3{invViewMatrix_*glm::vec4{0, 0, -1, 0}})  ;}
    // {0, 0, -1, 0} is the camera's forward direction
    glm::vec3 up() const {return glm::normalize(   glm::vec3{invViewMatrix_*glm::vec4{0, -1, 0, 0}})   ;}
    
    //getter
    virtual glm::mat4 getViewMatrix() const override {return viewMatrix_;}
    virtual glm::mat4 getProjMatrix(const float ratio) const override;
    virtual glm::vec3 getPosition() const override        {return  eye();}    
    
    void onMouseButton(int button, int action, double x, double y);
    void onCursorPos (double x, double y, GLFWwindow* window);
    void recordDragStart(double x, double y);
    
    
    ~JCameraPositioner_Arcball(){};
    

    void updateCamera();
    glm::quat rotation_{1.0f, 0.f , 0.f, 0.f};
    glm::mat4 viewMatrix_{1.0f}; //full camera transformation -> view matrix
    glm::mat4 invViewMatrix_{1.0f}; // inverse camera to find camera position, world space rotation exis

    glm::mat4 pivot_translation_; // inverse translation that moves your chosen pivot to the world origin
    //brings the pivot point to the origin

    glm::mat4 translation_;   // how far the camera is offset from the arcball pivot

private:

    glm::vec3 eye_  ;
    glm::vec3  pivot_{0.0f};

    DragMode dragMode_        = DragMode::None;
       
    glm::vec3 up_    ;

    float zoom_speed_   = 1.f;
    float pan_speed_    = 1.f;
    float orbit_speed_  =1.f;
    
    bool ifFirstPos_ = true;
    glm::vec2 preMousePos_;
    glm::vec2 currMousePos_;
    
    glm::vec2 dragStartMousePos2D_;
    glm::vec3 dragStartPos_;

    glm::quat screen_to_arcball(const glm::vec2& pos);


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
        virtual glm::mat4 getProjMatrix(const float ratio) const override;
        //setter
        void setCameraPosition(const glm::vec3& pos)        {cameraPosition_ = pos;}
        void setSpeed(const glm::vec3& speed)               { moveSpeed_ = speed ;}
        void setUPvector(const glm::vec3& up);
        void resetMousePosition(const glm::vec3& newPos)    {mousePos_ = newPos;}
        
        void onMouseButton(int button, int action, double x, double y);
        void onKeyboardButton(int button, int action);
    
    
    
        ~JCameraPositioner_firstPerson(){};
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

    private:
        glm::vec3 cameraPosition_               = glm::vec3( 0.0f, 10.0f, 10.0f);
        glm::quat cameraOrientation_            = glm::quat(glm::vec3(0));         //glm::quat(euler angles) = identityQuat (1.0f, 0.0f, 0.0f, 0.0f)
    
        glm::vec2 mousePos_                     = glm::vec2(0);
        glm::vec3 moveSpeed_                   = glm::vec3(0.0f);
    
        glm::vec3 up_                           = glm::vec3(0.0f, 0.0f, 1.0f);

        InputMode inputMode_        = InputMode::None;

    
    };
    




 //--------------------------
class JCamera final{

    public:
        explicit JCamera(JCameraPositioner& positioner): positioner_(&positioner) {}
        NO_COPY(JCamera);
    
        glm::mat4 getViewMatrix() const     {return positioner_->getViewMatrix();}
        glm::vec3 getPosition() const       {return positioner_->getPosition();}
        glm::mat4 getProjMatrix(const float ratio) const {
            std::cerr << "DEBUG: JCamera::getProjMatrix called, positioner_ = " << positioner_ << std::endl;
            assert (positioner_ && "positioner is null in getProjMatrix!");
            if(!positioner_){
                std::cerr << "ERROR: positioner_ is null!" << std::endl;
                return glm::mat4{1.f};
            }
            std::cerr << "DEBUG: About to call positioner_->getProjMatrix()" << std::endl;
            auto result = positioner_->getProjMatrix(ratio);
            std::cerr << "DEBUG: positioner_->getProjMatrix() returned successfully" << std::endl;
            return result;
        }
        const JCameraPositioner* debug_getRawPositioner() const { return positioner_; }
    
    private:
        const JCameraPositioner* positioner_;  //指向const cameraPosition的指针，通过这个指针不可修改被指向的对象
        glm::mat4 proj_;
    
    };
    
    

}


