#ifndef __FLEXR_AR_VIRTUAL_WORLD__
#define __FLEXR_AR_VIRTUAL_WORLD__

#include <bits/stdc++.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#endif

#include "types/gl/model.h"
#include "types/gl/camera.h"
#include "types/ar/virtual_object.h"
#include "utils/path_finder.h"
#include "defs.h"

namespace flexr
{
  namespace ar_types {

    class VirtualWorld {
    public:
      int index;
      glm::mat4 worldMat;
      glm::mat4 projection;
      flexr::gl_types::Camera camera;
      std::vector<flexr::gl_types::Model> models;
      std::vector<flexr::ar_types::VirtualObject> objects;

      VirtualWorld(int index, int width=1280, int height=720) : index(index), camera(glm::vec3(0.0f, 3.0f, 0.0f)) {
        worldMat = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(camera.zoom), (float)width/(float)height, 0.1f, 100.0f);
      }

      void addModel(const char* path) {
        debug_print("%s", path);
        std::string strPath(path);
        flexr::gl_types::Model newModel(flexr::utils::PathFinder::find(strPath));
        models.push_back(newModel);
      }

      void addObject(flexr::ar_types::VirtualObject &newObject) {
        objects.push_back(newObject);
      }

      void emptyObject() {
        objects.clear();
      }

      void resetCoord() {
        worldMat = glm::mat4(1.0f);
      }

      void translate(glm::vec3 trans) {
        worldMat = glm::translate(worldMat, trans);
      }

      void rotate(glm::vec3 rot) {
        worldMat = glm::rotate(worldMat, rot.x, glm::vec3(1, 0, 0));
        worldMat = glm::rotate(worldMat, rot.y, glm::vec3(0, 1, 0));
        worldMat = glm::rotate(worldMat, rot.z, glm::vec3(0, 0, 1));
      }

      void scale(glm::vec3 scale = glm::vec3(0.5, 0.5, 0.5)) {
        worldMat = glm::scale(worldMat, scale);
      }

      void draw(flexr::gl_types::Shader &shader) {
        glm::mat4 modelMat;

        //shader.setMat4("model", worldMat);
        std::vector<flexr::ar_types::VirtualObject>::iterator objIter;
        for(objIter = objects.begin(); objIter != objects.end(); ++objIter) {
          modelMat = worldMat * objIter->objectMat;
          shader.setMat4("model", modelMat);

          models[objIter->modelIndex].draw(shader);
        }
      }
    };

  } // namespace types
} // namespace flexr

#endif

