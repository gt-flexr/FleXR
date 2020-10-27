#ifndef __MXRE_COMPLEX_YOLO_RENDERER__
#define __MXRE_COMPLEX_YOLO_RENDERER__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "cv_types.h"
#include "utils/path_finder.h"
#include "perceptions/complex_yolo/types.h"

#include "gl/egl_utils.h"
#include "gl/gl_utils.h"
#include "gl/gl_objects.h"
#include "gl/gl_types.h"
#include "gl/camera.h"
#include "gl/shader.h"
#include "gl/model.h"

#include "ar/world.h"
#include "ar/object.h"

namespace mxre
{
  namespace pipeline
  {
    namespace rendering
    {
      class ComplexYOLORenderer : public raft::kernel
      {
      private:
        mxre::egl::EGLPbuffer *pbuf;
        GLuint backgroundTexture;
        mxre::gl::Camera camera;
        mxre::gl::Shader shader;
        mxre::ar::World worldMap;
        bool binding;
        int width, height, depth;
      public:
        ComplexYOLORenderer(int width = 1252, int height = 380, int depth=50);
        ~ComplexYOLORenderer();
        virtual raft::kstatus run();
      };
    } // namespace rendering
  }   // namespace pipeline
} // namespace mxre

#endif

