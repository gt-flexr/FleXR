#ifndef __EGL_UTILS__
#define __EGL_UTILS__

#include <EGL/egl.h>
#include <opencv2/opencv.hpp>
#include "defs.h"


namespace mxre
{
  namespace egl
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

    static void initEGLPbuffer(EGLPbuffer &pbuf)
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

    static void bindPbuffer(EGLPbuffer &pbuf) {
      if (eglMakeCurrent(pbuf.eglDpy, pbuf.eglSurf, pbuf.eglSurf, pbuf.eglCtx) == EGL_FALSE)
        debug_print("Failed to bind");
    }

    static void unbindPbuffer(EGLPbuffer &pbuf) {
      if (eglMakeCurrent(pbuf.eglDpy, NULL, NULL, NULL) == EGL_FALSE)
        debug_print("Failed to unbind");
    }

    static void terminatePbuffer(EGLPbuffer &pbuf) {
      eglTerminate(pbuf.eglDpy);
    }
  } // namespace egl
} // namespace mxre
#endif

