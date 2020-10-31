#ifndef __MXRE_OBJ_RENDERER__
#define __MXRE_OBJ_RENDERER__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "types/cv/types.h"

#include "utils/egl_utils.h"
#include "utils/gl_utils.h"
#include "types/gl/types.h"
#include "types/gl/camera.h"
#include "types/gl/shader.h"
#include "types/gl/model.h"

#include "types/ar/virtual_world_manager.h"
#include "types/clock_types.h"

namespace mxre
{
  namespace kernels
  {

    class ObjectRenderer : public raft::kernel
    {
    private:
      mxre::egl_types::pbuffer *pbuf;
      mxre::ar_types::VirtualWorldManager worldManager;
      GLuint backgroundTexture;
      bool binding;
      int width, height;

    public:
      ObjectRenderer(std::vector<mxre::cv_types::ObjectInfo> registeredObjs, int width, int height);
      ~ObjectRenderer();
      virtual raft::kstatus run();
    };

  }   // kernels
} // namespace mxre

#endif

