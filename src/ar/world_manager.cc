#include "ar/world_manager.h"

namespace mxre
{
  namespace ar
  {

    WorldManager::WorldManager() {
      numOfWorlds = 0;
      currentWorld = 0;
    }

    void WorldManager::initShader() {
      stbi_set_flip_vertically_on_load(true);
      shader.init(mxre::utils::PathFinder::find("build/examples/model_loading.vs").c_str(),
          mxre::utils::PathFinder::find("build/examples/model_loading.fs").c_str());
    }


    void WorldManager::addWorld(int index) {
      mxre::ar::World newWorld(index);
      printf("%dth world: register models", index);
      int modelIndex = index % 5;
      switch(modelIndex) {
        case 0:
          newWorld.addModel("resources/0_stone/Stone.obj");
          break;
        case 1:
          newWorld.addModel("resources/1_neck/Neck_Mech_Walker.obj");
          break;
        case 2:
          newWorld.addModel("resources/2_mars/Mars 2K.obj");
          break;
        case 3:
          newWorld.addModel("resources/3_earth/Earth 2K.obj");
          break;
        case 4:
          newWorld.addModel("resources/4_shuttle/Transport Shuttle_obj.obj");
          break;
      }
      worlds.push_back(newWorld);
      ++numOfWorlds;
    }


    void WorldManager::addObjectToWorld(int worldIndex) {
      int objectIndex = worlds[worldIndex].objects.size();
      mxre::ar::Object newObject(objectIndex);
      worlds[worldIndex].addObject(newObject);
    }


    void WorldManager::handleKeystroke(char key) {
      // handle the keystroke to select the world
      if(key >= 48 && key <= 48 + MAX_OBJS) {
        std::cout << "set world as " << key << "th" << std::endl;
        currentWorld = key - 48;
      }
      // handle the keystroke to move the world camera
      std::cout << currentWorld << "th world key " << key << std::endl;
      switch(key) {
        case 'r': case 'R': // camera reset
          worlds[currentWorld].camera.resetCamera();
          break;
        case 'w': case 'W':
          worlds[currentWorld].camera.processTranslation(gl::FORWARD);
          break;
        case 's': case 'S':
          worlds[currentWorld].camera.processTranslation(gl::BACKWARD);
          break;
        case 'a': case 'A':
          worlds[currentWorld].camera.processTranslation(gl::LEFT);
          break;
        case 'd': case 'D':
          worlds[currentWorld].camera.processTranslation(gl::RIGHT);
          break;

        case 'i': case 'I': // pitch
          worlds[currentWorld].camera.processRotation(gl::FORWARD);
          break;
        case 'k': case 'K': // pitch
          worlds[currentWorld].camera.processRotation(gl::BACKWARD);
          break;
        case 'j': case 'J': // yaw
          worlds[currentWorld].camera.processRotation(gl::LEFT);
          break;
        case 'l': case 'L': // yaw
          worlds[currentWorld].camera.processRotation(gl::RIGHT);
          break;
      }
    }


    void WorldManager::startWorlds(char key, std::vector<mxre::gl::ObjectContext> &objCtxs) {
      handleKeystroke(key);

      shader.use();
      std::vector<mxre::gl::ObjectContext>::iterator objCtxIter;
      for (objCtxIter = objCtxs.begin(); objCtxIter != objCtxs.end(); ++objCtxIter) {
        int detectedWorldIdx = objCtxIter->index;
        shader.setMat4("projection", worlds[detectedWorldIdx].projection);
        shader.setMat4("view", worlds[detectedWorldIdx].camera.getViewMatrix());

        worlds[detectedWorldIdx].resetCoord();
        worlds[detectedWorldIdx].translate(objCtxIter->tvec);
        worlds[detectedWorldIdx].rotate(objCtxIter->rvec);
        worlds[detectedWorldIdx].scale(glm::vec3(0.1, 0.1, 0.1));
        worlds[detectedWorldIdx].draw(shader);
        glFlush();
      }
      glPopMatrix();
      glUseProgram(0);
    }

  } // namespace ar
} // namespace mxre

