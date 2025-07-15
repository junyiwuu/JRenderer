#include "device.hpp"


JDevice::JDevice(JWindow& window):window_app(window){
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    checkDriverProperties();
    createLogicalDevice();
    createCommandPool();
}


JDevice::~JDevice(){
    vkDestroyCommandPool(device_, commandPool_, nullptr);
    vkDestroyDevice(device_, nullptr);

    if (enableValidationLayers){
        DestroyDebugUtilsMessengerEXT(instance_, debugMessenger, nullptr);}

    vkDestroySurfaceKHR(instance_, surface_, nullptr);
    vkDestroyInstance(instance_, nullptr);
    
}





static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(   // macros, can be extended to support cross-platform
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData  )
    {
        std::cerr<< "Validation layer: " << pCallbackData->pMessage << std::endl;  // get pMessage from pCallbackData (struct), since it is a pointer so use arrow
        return VK_FALSE; //false means meet validation layer but not stop the app
    }


VkResult JDevice::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}


void JDevice::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


void JDevice::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo){
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}




// check if validation layer is supported
bool JDevice::checkValidationLayerSupport(){
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr); // check how many isntance layer that current env/driver support

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

     //check if the layer that you want to use are supported by current env/driver
    for(const char* layerName: validationLayers){
        bool layerFound=false;
        for(const auto& layerProperties: availableLayers){
            if (strcmp(layerName, layerProperties.layerName) == 0 ){
                layerFound = true;
                break; }
        }
        if (!layerFound){ return false; }
    }
    return true;
}


void JDevice::createInstance(){
    if (enableValidationLayers && !checkValidationLayerSupport()){
        throw std::runtime_error("validation layers requested, but not available!");
    }
    // appinfo , info that show to driver
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); //you make this number
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    //create vulkan instance
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

            //include validation layer names if they are enabled
    if (enableValidationLayers){  // for now, transfer the validation layer, can be some other layers
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }else{
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    // glfw extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    // get all required extensions
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // extensions
    uint32_t extensionCount=0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr); // ask how many extensions you have, and write into extensionCount
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    // print out all avaliable extensions
    std::cout << "available extensions: \n";
    for (const auto& extension: availableExtensions){ //get extensions from vkEnumerateInstanceExtensionProperties
        std::cout << '\t' << extension.extensionName << "\n";  }

    //check if support glfw extensions
    for (uint32_t i=0; i<glfwExtensionCount; i++){
        bool found=false;
        for (const auto& ext: availableExtensions){
            if (strcmp(glfwExtensions[i], ext.extensionName) == 0){
                found=true;
                break;
            }}
        if(!found){ throw std::runtime_error(std::string("Missing required GLFW extension"));  }
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);
    if (result != VK_SUCCESS){
        throw std::runtime_error("failed to create instance!");
    }
}



void JDevice::setupDebugMessenger(){
    if (!enableValidationLayers) return; // if there is no validation layer, then dont need to go further

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};    //EXT: extension  // {} means set all values as default or 0
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger ) != VK_SUCCESS){
        throw std::runtime_error("failed to set up debug messenger!");
    }
}


// Window surface
void JDevice::createSurface(){
    if (glfwCreateWindowSurface(instance_, window_app.getGLFWwindow(), nullptr, &surface_) != VK_SUCCESS ){
        throw std::runtime_error("failed to create a window surface!");
    }
}



VkSampleCountFlagBits JDevice::getMaxUsableSampleCount(){
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice_, &physicalDeviceProperties);
    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    if(counts & VK_SAMPLE_COUNT_64_BIT){return VK_SAMPLE_COUNT_64_BIT; }
    if(counts & VK_SAMPLE_COUNT_32_BIT){return VK_SAMPLE_COUNT_32_BIT; }
    if(counts & VK_SAMPLE_COUNT_16_BIT){return VK_SAMPLE_COUNT_16_BIT; }
    if(counts & VK_SAMPLE_COUNT_8_BIT){return VK_SAMPLE_COUNT_8_BIT; }
    if(counts & VK_SAMPLE_COUNT_4_BIT){return VK_SAMPLE_COUNT_4_BIT; }
    if(counts & VK_SAMPLE_COUNT_2_BIT){return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}


//Device
void JDevice::pickPhysicalDevice(){

    uint32_t deviceCount=0;
    vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

    if (deviceCount==0){
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    //if find any , allocate an array to hold all of the vkphysicaldevice handles
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

    //add any suitable device
    for (const auto& device: devices){
        if (isDeviceSuitable(device)){
            physicalDevice_ = device; 
            msaaSamples_ = getMaxUsableSampleCount();
            break; } 
                    
    
    }

    if (physicalDevice_ == VK_NULL_HANDLE){ throw std::runtime_error("failed to find a suitable GPU!");}


    //find the GPU which has the highest score
    std::multimap<int, VkPhysicalDevice> candidates;
    for (const auto& device: devices){
        int score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }
    if(candidates.rbegin()->first > 0){
        physicalDevice_ = candidates.rbegin()->second;
    }else{
        throw std::runtime_error("failed to find a suitable GPU!");
    }

}



// Logical device
void JDevice::createLogicalDevice(){
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value() , indices.presentFamily.value()};   // remove duplicated
    //得到的set里面的数字代表的是其中至少一个是带graphic的一个是带present的
    
    // queue priority must be set. Most of time for basic render, just one queue
    float queuePriority = 1.0f;
    for (uint32_t queueFamily: uniqueQueueFamilies){
        //queue
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
 

    //what device features will be use for logical device, then driver can turn on these features
    VkPhysicalDeviceFeatures deviceFeatures{}; // for now, all false (default)
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    //logical device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
        // enable swapchain extension here
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if(vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS){
        throw std::runtime_error("failed to create logical device!");
    }
    
    //create queue -- the queues are automatically created along with the logical device
    vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
}






// Device Helplers
bool JDevice::isDeviceSuitable(VkPhysicalDevice device){
    VkPhysicalDeviceProperties deviceProperties;  //declare a struct
    vkGetPhysicalDeviceProperties(device, &deviceProperties); //copy the info
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if(extensionsSupported){
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    //check if support anistropy
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    // run basic checking
    return indices.isComplete()&& extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}


bool JDevice::checkDeviceExtensionSupport(VkPhysicalDevice device){
    uint32_t extensionCount;
    //check what extensions current device supported
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    // collect you required extensions
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    // if match, remove it from required extensions
    for(const auto& extension: availableExtensions){
        requiredExtensions.erase(extension.extensionName); }

    // if empty, means all matched, means all support, return true
    return requiredExtensions.empty();
}

void JDevice::checkDriverProperties(){
    if(checkDeviceExtensionSupport(physicalDevice_) == true ){
        VkPhysicalDeviceProperties2 deviceProperties2 = {};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = &driverProperties_;
        driverProperties_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
        vkGetPhysicalDeviceProperties2(physicalDevice_, &deviceProperties2);

    }
    // else{ std::cout << "this is worng"<< std::endl;}
}


// Queue family
QueueFamilyIndices JDevice::findQueueFamilies(VkPhysicalDevice device){
    QueueFamilyIndices indices;
    //first check the length
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    // then check the contents
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    //give each queue family an index
    int i = 0;
    for (const auto& queueFamily: queueFamilies){
        // check if support graphic
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphicsFamily = i;
        }
        //check present
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
        if(presentSupport){
            indices.presentFamily=i;
        }
        // if both satisfied
        if(indices.isComplete()){
            break; //early exit, as soon as find one graphic family
        }
        i++;
    }
    return indices;
}


// swapchain
SwapChainSupportDetails JDevice::querySwapChainSupport(VkPhysicalDevice device){
    SwapChainSupportDetails details;
    //check capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);
    //check surface formats
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);
    if(formatCount!=0){
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
    }
    //check present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);
    if(presentModeCount!=0){
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());}

    return details;
}




int JDevice::rateDeviceSuitability(VkPhysicalDevice device){
    VkPhysicalDeviceProperties deviceProperties;  
    vkGetPhysicalDeviceProperties(device, &deviceProperties); 
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    int score=0;
    //discrete GPU high score
    if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){  score += 1000; }

    score += deviceProperties.limits.maxImageDimension2D;  // support bigger 2d image resolution, score higher
    //cant function without geometry shader
    if(!deviceFeatures.geometryShader){ return 0;}
    
    return score;
}





// return the required list of extensions based on whether validation layers are enabled or not
std::vector<const char*> JDevice::getRequiredExtensions(){
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // copy this range into extensions vector
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    // if you need use validation layer, then add debug extension
    if(enableValidationLayers){
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

    return extensions;
}





VkFormat JDevice::findSupportFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features){
    for(VkFormat format : candidates){
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice_, format, &props);

        if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures&features) == features){
            return format;
        }else if(tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures&features) == features){
            return format;}
    }
    throw std::runtime_error("failed to find supported format!");
}








VkResult JDevice::createImageViewWithInfo(
    const VkImageViewCreateInfo& imageViewInfo, VkImageView& imageView ){

    return vkCreateImageView(device_, &imageViewInfo, nullptr, &imageView);
    
}


VkResult JDevice::createImageWithInfo(const VkImageCreateInfo &imageInfo, 
    VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    // create the image
    if(vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS){
        throw std::runtime_error("failed to create image!");}

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device_, image, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = util::findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice_);
    VkResult res = vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory);
    vkBindImageMemory(device_, image, imageMemory, 0);
    return res;
}




//command pool
void JDevice::createCommandPool(){
    QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if(vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");}
}




//tell if chosen depth format contains a stencil component
bool JDevice::hasStencilComponent(VkFormat format){
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}



VkFormat JDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice_, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}









void JDevice::transitionImageLayout( VkCommandBuffer commandBuffer, VkImage image,  
    VkImageLayout oldLayout, VkImageLayout newLayout,
    VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, 
    VkImageAspectFlags aspectMask,  uint32_t mipLevels){

    // VkCommandBuffer commandBuffer = util::beginSingleTimeCommands(device_, commandPool_);
    
    VkImageMemoryBarrier barrier{};  //barrier是要求在之前的操作全部结束后才能用心的layout去操作图片
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    switch(oldLayout)
    {
        case VK_IMAGE_LAYOUT_UNDEFINED:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        barrier.srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image is a transfer source
        // Make sure any reads from the image have been finished
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image will be used as a transfer destination
        // Make sure any writes to the image have been finished
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (barrier.srcAccessMask == 0)
        {
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    
    // if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout==VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
    // barrier.srcAccessMask = 0;
    // barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    // srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    // dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;}

    // else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){
    // barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    // barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    // srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    // dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;}

    // else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
    // barrier.srcAccessMask = 0;
    // barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    // srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;  // the earliest stage of the pipeline (the enrty of the pipeline)
    // dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;} // 指在片元（像素）着色前，GPU 会做深度、模板测试等早期操作。等到 GPU 进入早期片元测试前，一定要保证 barrier 已经生效（比如 image layout 已经完成转换）

    // else{
    // throw std::invalid_argument("unsupported layout transition!");}

    // if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL){
    // barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // if(hasStencilComponent(format)){
    // barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;}  // first OR then assign the value. this way, if aspectMask already have value, add one more stencil
    // }else{
    // barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;  }


    vkCmdPipelineBarrier(
    commandBuffer,
    srcStage, dstStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
    );
    // util::endSingleTimeCommands(device_, commandBuffer, commandPool_, graphicsQueue_ );
}
