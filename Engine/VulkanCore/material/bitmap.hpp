

#pragma once
#include <vulkan/vulkan.hpp>
#include <string.h>
#include <vector>
#include <iostream>

#include <glm/glm.hpp>



enum eJBitmapType{
    eJBitmapType_2D,
    eJBitmapType_Cube,
};

enum eJBitmapFormat{
    eJBitmapFormat_UnsignedByte,
    eJBitmapFormat_Float,
};


struct JBitmap
{

    JBitmap() = default;
    JBitmap(int w, int h,            int channels, eJBitmapFormat format);
    JBitmap(int w, int h, int depth, int channels, eJBitmapFormat format);
    JBitmap(int w, int h,            int channels, eJBitmapFormat format, const void* ptr);

    int w_          = 0;
    int h_          = 0;
    int depth_      = 1;
    int channels_    = 3;
    
    eJBitmapFormat format_ = eJBitmapFormat_UnsignedByte;
    eJBitmapType   type_   = eJBitmapType_2D;
    std::vector<uint8_t> data_;


// Helpler function
    static int getBytesPerChannel(eJBitmapFormat format){
        if(format == eJBitmapFormat_UnsignedByte) return 1; //unsigned byte is 1 byte (8bits)
        if(format == eJBitmapFormat_Float)        return 4; //float is 4 bytes (32 bits)
        return 0;
    }
    void setPixel(int x, int y, const glm::vec4& color){
        (*this.*setPixelFunc)(x, y , color);
    }
    glm::vec4 getPixel(int x, int y) const{
        return ((*this.*getPixelFunc)(x, y));
    }
    VkFormat getVkFormat() const { return bitmapFormatToVkFormat(format_, channels_);}




private:
    using setPixel_t = void(JBitmap::*)(int, int, const glm::vec4&); //for pointer who has (int, int , vec4) and no return 
    using getPixel_t = glm::vec4 (JBitmap::*)(int, int) const;

    setPixel_t setPixelFunc = &JBitmap::setPixelUnsignedByte;;
    getPixel_t getPixelFunc = &JBitmap::getPixelUnsignedByte;

    void initGetSetFuncs(){
        switch (format_)
        {
        case eJBitmapFormat_UnsignedByte:
            setPixelFunc = &JBitmap::setPixelUnsignedByte;
            getPixelFunc = &JBitmap::getPixelUnsignedByte;
            break;
        
        case eJBitmapFormat_Float:
            setPixelFunc = &JBitmap::setPixelFloat;
            getPixelFunc = &JBitmap::getPixelFloat;
            break;
        }
    }



    void setPixelFloat(int x, int y, const glm::vec4& color){
        if (!(x >= 0 && x < w_ && y >= 0 && y < h_)) {
            std::cout << "setPixelFloat out of range: (" << x << ", " << y << ") size=("
                      << w_ << ", " << h_ << ")" << std::endl;
            throw std::out_of_range("setPixelFloat out of range");
        }
        
        const int offset = channels_ * (y*w_ + x);
        float* data = reinterpret_cast<float*>(data_.data());
        if(channels_ > 0) { data[offset+0] = color.r; }
        if(channels_ > 1) { data[offset+1] = color.g; }
        if(channels_ > 2) { data[offset+2] = color.b; }
        if(channels_ > 3) { data[offset+3] = color.a; }
    }

	glm::vec4 getPixelFloat(int x, int y) const
	{
        // assert(x>=0 && x<w_ && y>=0 && y<h_&& "get pixel float out of range" ); 
        if (!(x >= 0 && x < w_ && y >= 0 && y < h_)) {
            std::cout << "getPixelFloat out of range: (" << x << ", " << y << ") size=("
                      << w_ << ", " << h_ << ")" << std::endl;
            throw std::out_of_range("getPixelFloat out of range");
        }
        
		const int offset = channels_ * (y*w_ + x);
		const float* data = reinterpret_cast<const float*>(data_.data());
		return glm::vec4(
			channels_ > 0 ? data[offset + 0] : 0.0f,
			channels_ > 1 ? data[offset + 1] : 0.0f,
			channels_ > 2 ? data[offset + 2] : 0.0f,
			channels_ > 3 ? data[offset + 3] : 0.0f);
	}

    //rgbrgbrgb, not rrrgggbbb. stbi use interleaved (rgbrgbrgb)
    void setPixelUnsignedByte(int x, int y, const glm::vec4& color){
        const int offset = channels_ * (y*w_ + x);
        if (channels_ > 0) { data_[offset + 0] = uint8_t(color.r * 255.f); }
        if (channels_ > 1) { data_[offset + 1] = uint8_t(color.g * 255.f); }
        if (channels_ > 2) { data_[offset + 2] = uint8_t(color.b * 255.f); }
        if (channels_ > 3) { data_[offset + 3] = uint8_t(color.a * 255.f); }
    }

    glm::vec4 getPixelUnsignedByte(int x, int y) const{
        const int offset = channels_ * (y * w_ + x);
        return glm::vec4(
            channels_ > 0 ? float(data_[offset + 0]) / 255.f : 0.f,
            channels_ > 1 ? float(data_[offset + 1]) / 255.f : 0.f,
            channels_ > 2 ? float(data_[offset + 2]) / 255.f : 0.f,
            channels_ > 3 ? float(data_[offset + 3]) / 255.f : 0.f  );
    }


    VkFormat bitmapFormatToVkFormat(eJBitmapFormat format, int channel) const
    {
        switch (format) {
        case eJBitmapFormat_UnsignedByte:
            switch (channel) {
            case 1: return VK_FORMAT_R8_UNORM;
            case 2: return VK_FORMAT_R8G8_UNORM;
            case 3: return VK_FORMAT_R8G8B8_UNORM;        // 注意有些实现可能不支持 3 通道，这时可以改成 VK_FORMAT_R8G8B8A8_UNORM 并在上传时填充 A=255
            case 4: return VK_FORMAT_R8G8B8A8_UNORM;
            default: break;
            }
            break;

        case eJBitmapFormat_Float:
            switch (channel) {
            case 1: return VK_FORMAT_R32_SFLOAT;
            case 2: return VK_FORMAT_R32G32_SFLOAT;
            case 3: return VK_FORMAT_R32G32B32_SFLOAT;
            case 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
            default: break;
            }
            break;
    }

    // if no compatibale
    return VK_FORMAT_UNDEFINED;
    }
};







































































































