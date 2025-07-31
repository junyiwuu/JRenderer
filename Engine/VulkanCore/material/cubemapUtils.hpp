#pragma once

#include <glm/glm.hpp>
// #include <algorithm>
#include "bitmap.hpp"


glm::vec3 faceCoordsToXYZ(int i, int j, int faceID, int faceSize);


JBitmap convertEquirectangularMapToVerticalCross (const JBitmap& bitmap);


  /*
        ------
        | +Y |
   ----------------
   | -X | -Z | +X |
   ----------------
        | -Y |
        ------
        | +Z |
        ------
  */

JBitmap convertVerticalCrossToCubeMapFaces(const JBitmap& bitmap);







