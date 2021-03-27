#ifndef __MXRE_FFMPEG_DECODER__
#define __MXRE_FFMPEG_DECODER__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <raft>
#include <zmq.hpp>

#include "defs.h"
#include "kernels/kernel.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/frame.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

namespace mxre
{
  namespace kernels
  {
    /* Class Deifinition */
    class FFmpegDecoder : public MXREKernel
    {
      private:
        std::string decoderName;
        int width, height;

        AVCodec *decoder;
        AVCodecContext *decoderContext;
        AVFrame *decodingFrame;
        uint8_t *decodingFrameBuffer;
        int decodingFrameSize;


        cv::Mat yuvFrame;

      public:
        FFmpegDecoder(std::string decoderName, int width, int height);
        ~FFmpegDecoder();
        virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

