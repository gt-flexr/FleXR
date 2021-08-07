#ifndef __FLEXR_FRAME_CONVERTER__
#define __FLEXR_FRAME_CONVERTER__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>

#include "defs.h"
#include "utils/cv_utils.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"

#include "kernels/kernel.h"

namespace flexr
{
  namespace kernels
  {
    using namespace flexr::types;


    /**
     * @brief Kernel to convert the frame into another pixel format
     *
     * Port Tag             | Type
     * ---------------------| ----------------------------
     * in_frame             | @ref flexr::types::Message<@ref flexr::types::Frame>
     * out_frame            | @ref flexr::types::Message<@ref flexr::types::Frame>
    */
    class FrameConverter : public FleXRKernel
    {
      public:
        enum Conversion { RGB2RGBA,
                          RGBA2RGB,
                          BGRA2RGB };


      /**
       * @brief Initialize kernel with conversion info
       * @param id
       *  Kernel ID
       * @param width
       *  Frame width
       * @param height
       *  Frame height
       * @param conv
       *  Pixel conversion type
       */
        FrameConverter(std::string id, int width, int height, Conversion conv);


        raft::kstatus run() override;


      private:
        int width, height;
        cv::Mat inFormat;
        int inFrameSize;
        Conversion conv;
    };

  }   // namespace kernels
} // namespace flexr

#endif

