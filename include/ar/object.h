#ifndef __MXRE_AR_OBJECT__
#define __MXRE_AR_OBJECT__

#include <bits/stdc++.h>
#include <glm/glm.hpp>
#include "gl/model.h"

namespace mxre
{
  namespace ar {
    class Object {
    public:
      int modelIndex;
      glm::mat4 objectMat;
      Object() {
        modelIndex = 0;
        objectMat = glm::mat4(1.0f);
      }

      Object(int modelIndex) : modelIndex(modelIndex) {
        objectMat = glm::mat4(1.0f);
      }
    };
  } // namespace ar
} // namespace mxre

#endif

