#include "utility.hpp"


namespace util{



    std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
    
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
    
        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
    
        file.seekg(0);
        file.read(buffer.data(), fileSize);
    
        file.close();
    
        return buffer;
    }



    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice){
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
         //检查typefilter这个二进制数的第i位是否为1
        for(uint32_t i=0; i<memProperties.memoryTypeCount; i++){
            if((typeFilter&(1<<i))&&
                (memProperties.memoryTypes[i].propertyFlags & properties)==properties) {  
                    return i;}
        } throw std::runtime_error("failed to find suitable memory type!");
    }
    
    
}




namespace tools{



    VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset ){
        vkMapMemory(device, memory, 0, size, 0, &mapped);  //staging buffer is host access on gpu

    }


    void Buffer::unmap(){

        if (mapped)
        {
            vkUnmapMemory(device, memory);
            mapped = nullptr;
        }
    }


    VkResult Buffer::bind(VkDeviceSize offset ){
        return vkBindBufferMemory(device, buffer, memory, offset);

    }


    void Buffer::memoryCopy(void* data, VkDeviceSize size){

        assert(mapped);
        memcpy(mapped, data, size);
    }


    VkResult Buffer::flush(VkDeviceSize size , VkDeviceSize offset ){
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(device, 1, &mappedRange);

    }


    VkResult Buffer::invalidate(VkDeviceSize size , VkDeviceSize offset){

        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = memory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
    }


    void Buffer::destroy(){

        if (buffer)
        {
            vkDestroyBuffer(device, buffer, nullptr);
        }
        if (memory)
        {
            vkFreeMemory(device, memory, nullptr);
        }
    }



    void Buffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
    {
        descriptorBufferInfo.offset = offset;
        descriptorBufferInfo.buffer = buffer;
        descriptorBufferInfo.range = size;
    }












}



        
        
        
        