
#pragma once
#include <vulkan/vulkan.hpp>

#include "global.hpp"
class JDevice;




class JSync{

public:
    JSync(JDevice& device);
    ~JSync();

    NO_COPY(JSync);


    //members, can getter
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence     inFlightFence;


private:
    JDevice& device_app;


};


