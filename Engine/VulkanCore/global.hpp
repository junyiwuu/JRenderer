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


























