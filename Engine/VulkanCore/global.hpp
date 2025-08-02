#pragma once
#include <vulkan/vulkan.hpp>
#include <cassert>
#include <iostream>


namespace Global{
	inline constexpr int MAX_FRAMES_IN_FLIGHT = 3;
	
	
	
	
	}

	


#define VK_CHECK_RESULT(f)																				\
do {																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << tools::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}while(0)


namespace tools
{
    extern bool errorModeSilent;
    extern std::string resourcePath;

    std::string errorString(VkResult errorCode);
}




#define NO_COPY(TypeName) 					\
	TypeName(const TypeName&) = delete;			\
	TypeName &operator=(const TypeName&) = delete; 		

// Forward declarations
class JWindow;
class InteractiveSystem;
struct GLFWwindow;

// Centralized GLFW callback manager
class AppContext {
private:
    JWindow* window_ = nullptr;
    InteractiveSystem* interactiveSystem_ = nullptr;
    
public:
    // Register components
    void setWindow(JWindow* window) { window_ = window; }
    void setInteractiveSystem(InteractiveSystem* interactive) { interactiveSystem_ = interactive; }
    
    // Setup all GLFW callbacks in one place
    void registerAllCallbacks(GLFWwindow* window);
    
    // Static callback functions that dispatch to appropriate objects
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double x, double y);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};


























