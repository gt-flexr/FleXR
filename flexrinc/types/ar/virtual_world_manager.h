#ifndef __FLEXR_AR_VIRTUAL_WORLD_MANAGER__
#define __FLEXR_AR_VIRTUAL_WORLD_MANAGER__

#include <bits/stdc++.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "types/gl/shader.h"
#include "types/gl/types.h"
#include "types/ar/virtual_world.h"
#include "types/ar/virtual_object.h"
#include "utils/path_finder.h"

namespace flexr
{
  namespace ar_types {

    class VirtualWorldManager {
    public:
      int numOfWorlds;
      int currentWorld;
      std::vector<flexr::ar_types::VirtualWorld> worlds;
      flexr::gl_types::Shader shader;

      VirtualWorldManager();
      void initShader();
      void addWorld(int index);
      void addObjectToWorld(int worldIndex);
      void handleKeystroke(char key);
      void startWorlds(char key, std::vector<flexr::gl_types::ObjectContext> objCtxs);
    };

  } // namespace ar
} // namespace flexr

#endif

