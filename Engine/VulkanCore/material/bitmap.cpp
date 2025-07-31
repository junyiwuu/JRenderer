#include "bitmap.hpp"



JBitmap::JBitmap(int w, int h, int channels, eJBitmapFormat format):
    w_(w), h_(h), channels_(channels), format_(format), data_(w*h*channels*getBytesPerChannel(format))
{
    initGetSetFuncs();}


JBitmap::JBitmap(int w, int h, int depth, int channels, eJBitmapFormat format):
    w_(w), h_(h), depth_(depth), channels_(channels), format_(format), data_(w*h*depth*channels*getBytesPerChannel(format))
{
    initGetSetFuncs();}


JBitmap::JBitmap(int w, int h, int channels, eJBitmapFormat format, const void* ptr):
    w_(w), h_(h), channels_(channels), format_(format), data_(w*h*channels*getBytesPerChannel(format))
{
    initGetSetFuncs();
    memcpy(data_.data(), ptr, data_.size());
    if(!data_.data()){     
        throw std::runtime_error("DEBUG: No data copied for Bitmap!");
    }
}