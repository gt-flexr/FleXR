#ifndef __MXRE_OBJ_RENDERER__
#define __MXRE_OBJ_RENDERER__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "egl_utils.h"
#include "gl_utils.h"
#include "gl_types.h"

namespace mxre
{
  namespace pipeline
  {
    namespace rendering
    {
      class ObjectRenderer : public raft::kernel
      {
      private:
        mxre::eglutils::EGLPbuffer pbuf;
        GLuint backgroundTexture;
      public:
        ObjectRenderer();
        ~ObjectRenderer();
        virtual raft::kstatus run();
      };
    } // namespace rendering
  }   // namespace pipeline
} // namespace mxre

#endif