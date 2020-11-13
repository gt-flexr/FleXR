#ifndef __EGL_TYPES__
#define __EGL_TYPES__

#include <EGL/egl.h>
#include <opencv2/opencv.hpp>
#include "defs.h"


namespace mxre
{
  namespace egl_types
  {

    static const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE};


    typedef struct pbuffer
    {
      EGLDisplay eglDpy;
      EGLint major, minor, numConfigs;
      EGLConfig eglCfg;
      EGLSurface eglSurf;
      EGLContext eglCtx;
    } pbuffer;

  } // namespace utils
} // namespace mxre
#endif

