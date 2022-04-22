#pragma once

#ifdef __FLEXR_KERNEL_FRAME_CONVERTER__

#include <opencv2/opencv.hpp>

#include "flexr_core/include/core.h"

namespace flexr
{
  namespace kernels
  {
    using namespace flexr::types;
    using FrameConverterInMsgType = types::Message<types::Frame>;
    using FrameConverterOutMsgType = types::Message<types::Frame>;


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
                          BGRA2RGB,
                          GRAY2RGB,
                          RGB2GRAY};


      /**
       * @brief Initialize kernel only with id
       * @param id
       *  Kernel ID
       */
        FrameConverter(std::string id);


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


      /**
       * @brief Set frame resolution to convert
       * @param width
       *  Frame width
       * @param height
       *  Frame height
       */
        void setResolution(int width, int height);


      /**
       * @brief Set conversion type with string
       * @param conv
       *  Conversion type string
       */
        void setConversion(std::string conversionType);


      /**
       * @brief Set conversion type with enumerator
       * @param conv
       *  Conversion type enumerator
       */
        void setConversion(Conversion conv);


        raft::kstatus run() override;


      private:
        int width, height;
        Conversion conv;

        cv::Mat inFormat;
        cv::Mat outFormat;
        int inFrameSize;
        int outFrameSize;

        void setFormats();
    };

  }   // namespace kernels
} // namespace flexr

#endif

