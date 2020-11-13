#ifndef __MXRE_OBJ_RENDERER__
#define __MXRE_OBJ_RENDERER__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "kernels/kernel.h"
#include "types/cv/types.h"

#include "utils/egl_utils.h"
#include "utils/gl_utils.h"
#include "types/gl/types.h"
#include "types/gl/camera.h"
#include "types/gl/shader.h"
#include "types/gl/model.h"

#include "types/ar/virtual_world_manager.h"
#include "types/clock_types.h"
#include "types/frame.h"

namespace mxre
{
  namespace kernels
  {

    class ObjectRenderer : public MXREKernel
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
      bool logic(mxre::types::Frame *inFrame, std::vector<mxre::gl_types::ObjectContext> *inObjContext, char inKey,
                 mxre::types::Frame *outFrame);
    };

  }   // kernels
} // namespace mxre

#endif

