#ifndef __EGL_UTILS__
#define __EGL_UTILS__

#include <EGL/egl.h>
#include <opencv2/opencv.hpp>
#include "defs.h"


namespace mxre
{
  namespace eglutils
  {
    static const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE};

    static const EGLint pbufferAttribs[] = {EGL_WIDTH, WIDTH,
                                            EGL_HEIGHT, HEIGHT,
                                            // EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
                                            // EGL_TEXTURE_TARGET, EGL_GL_TEXTURE_2D,
                                            EGL_NONE};

    typedef struct EGLPbuffer
    {
      EGLDisplay eglDpy;
      EGLint major, minor, numConfigs;
      EGLConfig eglCfg;
      EGLSurface eglSurf;
      EGLContext eglCtx;
    } EGLPbuffer;

    void initEGLPbuffer(EGLPbuffer &pbuf)
    {
      // 1. get & init EGL Display
      pbuf.eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      eglInitialize(pbuf.eglDpy, &(pbuf.major), &(pbuf.minor));

      eglChooseConfig(pbuf.eglDpy, configAttribs, &(pbuf.eglCfg), 1, &(pbuf.numConfigs));

      // 3. Create a surface
      pbuf.eglSurf = eglCreatePbufferSurface(pbuf.eglDpy, pbuf.eglCfg, pbufferAttribs);

      // 4. Bind the API
      eglBindAPI(EGL_OPENGL_API);

      // 5. Create a context and make it current
      pbuf.eglCtx = eglCreateContext(pbuf.eglDpy, pbuf.eglCfg, EGL_NO_CONTEXT, NULL);
    }

    void setCurrentPbuffer(EGLPbuffer &pbuf) {
      eglMakeCurrent(pbuf.eglDpy, pbuf.eglSurf, pbuf.eglSurf, pbuf.eglCtx);
    }

    void terminatePbuffer(EGLPbuffer &pbuf) {
      eglTerminate(pbuf.eglDpy);
    }
  } // namespace utils
} // namespace mxre
#endif