#ifndef __MXRE_COMPLEX_YOLO_RENDERER__
#define __MXRE_COMPLEX_YOLO_RENDERER__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "types/cv/types.h"

#include "types/complex_yolo_types.h"

#include "utils/gl_utils.h"
#include "utils/egl_utils.h"
#include "utils/path_finder.h"

#include "types/gl/types.h"
#include "types/gl/camera.h"
#include "types/gl/shader.h"
#include "types/gl/model.h"

#include "types/ar/virtual_object.h"
#include "types/ar/virtual_world.h"

namespace mxre
{
  namespace kernels
  {

      class ComplexYOLORenderer : public raft::kernel
      {
      private:
        mxre::egl_types::pbuffer *pbuf;
        GLuint backgroundTexture;
        mxre::gl_types::Camera camera;
        mxre::gl_types::Shader shader;
        mxre::ar_types::VirtualWorld worldMap;
        bool binding;
        int width, height, depth;
      public:
        ComplexYOLORenderer(int width = 1252, int height = 380, int depth=50);
        ~ComplexYOLORenderer();
        virtual raft::kstatus run();
      };

  }   // namespace pipeline
} // namespace mxre

#endif

