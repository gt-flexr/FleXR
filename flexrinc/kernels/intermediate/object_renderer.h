#ifndef __FLEXR_OBJ_RENDERER__
#define __FLEXR_OBJ_RENDERER__

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
#include "types/frame.h"

namespace flexr
{
  namespace kernels
  {

    using ObjRendererInCtxType    = types::Message<std::vector<gl_types::ObjectContext>>;
    using ObjRendererInFrameType  = types::Message<types::Frame>;
    using ObjRendererInKeyType    = types::Message<char>;
    using ObjRendererOutFrameType = types::Message<types::Frame>;


    /**
     * @brief Kernel to render virtual objects on the detected markers
     *
     * Port Tag             | Type
     * ---------------------| ----------------------------
     * in_frame             | @ref flexr::types::Message<@ref flexr::types::Frame>
     * in_marker_contexts   | @ref flexr::types::Message<std::vector<@ref flexr::gl_types::ObjectContext>>
     * in_key               | @ref flexr::types::Message<char>
     * out_frame            | @ref flexr::types::Message<@ref flexr::types::Frame>
    */
    class ObjectRenderer : public FleXRKernel
    {
    private:
      flexr::egl_types::pbuffer *pbuf;
      flexr::ar_types::VirtualWorldManager worldManager;

      GLuint backgroundTexture;
      bool binding;
      int width, height;

    public:
      /**
       * @brief Initialize kernel with registered marker info
       * @param registeredMarkers
       *  Registered marker lists
       */
      ObjectRenderer(std::string id, int width, int height);


      ~ObjectRenderer();


      virtual raft::kstatus run();


      bool logic(ObjRendererInFrameType  *inFrame,
                 ObjRendererInKeyType    * inKey,
                 ObjRendererInCtxType    *inMarkerContexts,
                 ObjRendererOutFrameType *outFrame);
    };

  }   // kernels
} // namespace flexr

#endif

