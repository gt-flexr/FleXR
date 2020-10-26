#ifndef __MXRE_AR_OBJECT__
#define __MXRE_AR_OBJECT__

#include <bits/stdc++.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#endif

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

      void translate(glm::vec3 trans) {
        objectMat = glm::translate(objectMat, trans);
      }

      void rotate(glm::vec3 rot) {
        objectMat = glm::rotate(objectMat, rot.x, glm::vec3(1, 0, 0));
        objectMat = glm::rotate(objectMat, rot.y, glm::vec3(0, 1, 0));
        objectMat = glm::rotate(objectMat, rot.z, glm::vec3(0, 0, 1));
      }

      void scale(glm::vec3 scale = glm::vec3(0.1, 0.1, 0.1)) {
        objectMat = glm::scale(objectMat, scale);
      }
    };

  } // namespace ar
} // namespace mxre

#endif

