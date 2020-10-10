#ifndef __MXRE_AR_WORLD_MANAGER__
#define __MXRE_AR_WORLD_MANAGER__

#include <bits/stdc++.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gl/shader.h"
#include "gl/gl_types.h"
#include "ar/world.h"
#include "ar/object.h"
#include "utils/path_finder.h"

namespace mxre
{
  namespace ar {

    class WorldManager {
    public:
      int numOfWorlds;
      int currentWorld;
      std::vector<mxre::ar::World> worlds;
      mxre::gl::Shader shader;

      WorldManager();
      void initShader();
      void addWorld(int index);
      void addObjectToWorld(int worldIndex);
      void handleKeystroke(char key);
      void startWorlds(char key, std::vector<mxre::gl::ObjectContext> &objCtxs);
    };

  } // namespace ar
} // namespace mxre

#endif

