#ifndef __EGL_UTILS__
#define __EGL_UTILS__

#include <EGL/egl.h>
#include <opencv2/opencv.hpp>
#include "defs.h"
#include "types/gl/egl_types.h"


namespace flexr
{
  namespace egl_utils
  {
    using namespace egl_types;

    static void initPbuffer(pbuffer &pbuf, int width, int height)
    {
      // 1. get & init EGL Display
      pbuf.eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      eglInitialize(pbuf.eglDpy, &(pbuf.major), &(pbuf.minor));

      eglChooseConfig(pbuf.eglDpy, configAttribs, &(pbuf.eglCfg), 1, &(pbuf.numConfigs));

      // 3. Create a surface
      EGLint attrs[] = {EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE};
      pbuf.eglSurf = eglCreatePbufferSurface(pbuf.eglDpy, pbuf.eglCfg, attrs);

      // 4. Bind the API
      eglBindAPI(EGL_OPENGL_API);

      // 5. Create a context and make it current
      pbuf.eglCtx = eglCreateContext(pbuf.eglDpy, pbuf.eglCfg, EGL_NO_CONTEXT, NULL);
    }


    static void bindPbuffer(pbuffer &pbuf) {
      if (eglMakeCurrent(pbuf.eglDpy, pbuf.eglSurf, pbuf.eglSurf, pbuf.eglCtx) == EGL_FALSE)
        debug_print("Failed to bind");
    }


    static void unbindPbuffer(pbuffer &pbuf) {
      if (eglMakeCurrent(pbuf.eglDpy, NULL, NULL, NULL) == EGL_FALSE)
        debug_print("Failed to unbind");
    }


    static void terminatePbuffer(pbuffer &pbuf) {
      eglTerminate(pbuf.eglDpy);
    }

  } // namespace utils
} // namespace flexr

#endif

