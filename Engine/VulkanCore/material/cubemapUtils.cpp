#include "cubemapUtils.hpp"





glm::vec3 faceCoordsToXYZ(int i, int j, int faceID, int faceSize){
    //[-1, 1] range

    const float A = 2.0f * float(i) / faceSize;
    const float B = 2.0f * float(j) / faceSize;

    if(faceID == 0) return glm::vec3(   - 1.0f , A - 1.0f , B - 1.0f);
    if(faceID == 1) return glm::vec3( A - 1.0f ,   - 1.0f , 1.0f - B);
    if(faceID == 2) return glm::vec3(     1.0f , A - 1.0f , 1.0f - B);
    if(faceID == 3) return glm::vec3( 1.0f - A ,     1.0f , 1.0f - B);
    if(faceID == 4) return glm::vec3( B - 1.0f , A - 1.0f ,     1.0f);
    if(faceID == 5) return glm::vec3( 1.0f - B , A - 1.0f ,   - 1.0f);
}




JBitmap convertEquirectangularMapToVerticalCross (const JBitmap& bitmap){
    // convert equirectangular map to vertical cross (like box) , using 4-samples bilinear interpolation
    if(bitmap.type_ != eJBitmapType_2D) return JBitmap();

    const int faceSize = bitmap.w_ / 4;
    const int w = faceSize * 3;
    const int h = faceSize * 4;
    JBitmap result(w, h, bitmap.channels_, bitmap.format_);

    const glm::ivec2 kFaceOffsets[] = {
        glm::ivec2(faceSize    , faceSize * 3),
        glm::ivec2(0           , faceSize    ),
        glm::ivec2(faceSize    , faceSize    ),
        glm::ivec2(faceSize * 2, faceSize    ),
        glm::ivec2(faceSize    , 0           ),
        glm::ivec2(faceSize    , faceSize * 2)
    };

    const int clampW = bitmap.w_ - 1;
    const int clampH = bitmap.h_ - 1;

    for (int face = 0; face != 6; face++){
        for (int i = 0; i != faceSize ; i++){
            for (int j = 0; j != faceSize; j++){
                //spherical coordinates
                const glm::vec3 P = faceCoordsToXYZ(i , j, face, faceSize);
                const float R = hypot(P.x, P.y);
                const float theta = atan2(P.y, P.x);
                const float phi = atan2(P.z, R);
                //convert spherical image to floating-point pixel coordinates (notebook)
                const float Uf = float(2.0f * faceSize * (theta + M_PI) / M_PI);
                const float Vf = float(2.0f * faceSize * (M_PI / 2.f - phi) / M_PI);
                //4 samples for bilinear interpolation // 在上面取到的floating point附近的四个像素index
                const int U1 = glm::clamp( int(floor(Uf)) , 0 , clampW); // pixel index
                const int V1 = glm::clamp( int(floor(Vf)) , 0 , clampH);
                const int U2 = glm::clamp ( U1 + 1, 0, clampW);
                const int V2 = glm::clamp ( V1 + 1, 0, clampH);
				// fractional part  比例
                const float s = Uf - U1;
                const float t = Vf - V1;
                // fetch 4-samples
                const glm::vec4 A = bitmap.getPixel(U1, V1);
                const glm::vec4 B = bitmap.getPixel(U2, V1);
                const glm::vec4 C = bitmap.getPixel(U1, V2);
                const glm::vec4 D = bitmap.getPixel(U2, V2);
                //bilinear interpolation
                const glm::vec4 color = 
                    A * (1-s) * (1-t) + 
                    B * (s)   * (1-t) + 
                    C * (1-s) * (t)   +
                    D * (s)   * (t)   ;
                result.setPixel( i + kFaceOffsets[face].x  ,  j + kFaceOffsets[face].y  ,  color);
            }
        }
    };
    return result;
}




JBitmap convertVerticalCrossToCubeMapFaces(const JBitmap& bitmap){
    const int faceWidth  = bitmap.w_ / 3;
    const int faceHeight = bitmap.h_ / 4;

    //6 depths
    JBitmap cubemap(faceWidth, faceHeight, 6, bitmap.channels_, bitmap.format_);
    cubemap.type_ = eJBitmapType_Cube;

    const uint8_t* src = bitmap.data_.data();
    uint8_t* dst       = cubemap.data_.data();
    const int pixelSize = cubemap.channels_ * JBitmap::getBytesPerChannel(cubemap.format_);  // byte size

    for(int face = 0 ; face!=6; ++face){
        for(int j = 0; j!= faceHeight; ++j){
            for(int i = 0; i!= faceWidth; ++i){
                int x = 0;
                int y = 0;

                switch(face){
                    case 0: // +x
                      x = 2*faceWidth + i;
                      y = 1*faceHeight + j;
                      break;
                    case 1: // -x
                      x = i;
                      y = faceHeight + j;
                      break;
                    case 2: // +y
                      x = 1*faceWidth + i;
                      y = j;
                      break;
                    case 3: // -y
                      x = 1*faceWidth + i;
                      y = 2*faceHeight + j;
                      break;
                    case 4: // +z
                      x = faceWidth + i;
                      y = faceHeight + j;
                      break;
                    case 5: // -z
                      x = 2 * faceWidth - (i + 1);
                      y = bitmap.h_ - (j + 1);
                      break;          }
                    memcpy(dst, 
                           src + (y * bitmap.w_ + x) * pixelSize, 
                           pixelSize);
                    dst += pixelSize;
            }  }  }

    return cubemap;
}





















