#ifndef __MXRE_AR_WORLD__
#define __MXRE_AR_WORLD__

#include <bits/stdc++.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gl/model.h"
#include "ar/object.h"
#include "utils/path_finder.h"

namespace mxre
{
  namespace ar {
    class World {
    public:
      int index;
      glm::mat4 worldMat;
      std::vector<mxre::gl::Model> models;
      std::vector<mxre::ar::Object> objects;

      World(int index) : index(index) {
        worldMat = glm::mat4(1.0f);
      }

      void addModel(const char* path) {
        debug_print("%s", path);
        std::string strPath(path);
        mxre::gl::Model newModel(mxre::utils::PathFinder::find(strPath));
        models.push_back(newModel);
      }

      void addObject(mxre::ar::Object &newObject) {
        objects.push_back(newObject);
      }

      void resetCoord() {
        worldMat = glm::mat4(1.0f);
      }

      void translate(glm::vec3 &trans) {
        worldMat = glm::translate(worldMat, trans);
      }

      void rotate(glm::vec3 &rot) {
        worldMat = glm::rotate(worldMat, rot.x, glm::vec3(1, 0, 0));
        worldMat = glm::rotate(worldMat, rot.y, glm::vec3(0, 1, 0));
        worldMat = glm::rotate(worldMat, rot.z, glm::vec3(0, 0, 1));
      }

      void scale(glm::vec3 scale = glm::vec3(0.5, 0.5, 0.5)) {
        worldMat = glm::scale(worldMat, scale);
      }

      void draw(mxre::gl::Shader &shader) {
        shader.setMat4("model", worldMat);
        std::vector<mxre::ar::Object>::iterator objIter;
        for(objIter = objects.begin(); objIter != objects.end(); ++objIter) {
          // worldMat * objIter->objectMat
          // shader.setMat4("model", worldMat);
          models[objIter->modelIndex].draw(shader);
        }
      }
    };
  } // namespace ar
} // namespace mxre

#endif

