#ifndef __MXRE_FFMPEG_ENCODER__
#define __MXRE_FFMPEG_ENCODER__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <raft>
#include <zmq.hpp>
#include <ifaddrs.h>

#include "defs.h"
#include "kernels/kernel.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
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
    class FFmpegEncoder : public MXREKernel
    {
      private:
        std::string encoderName;
        int width, height;

        AVCodec *encoder;
        AVCodecContext *encoderContext;
        AVFrame *encodingFrame;

        cv::Mat yuvFrame;

      public:
        FFmpegEncoder(std::string encoderName, int width, int height, int bitrate, int fps);
        ~FFmpegEncoder();
        virtual raft::kstatus run();
    };

  } // namespace kernels
} // namespace mxre

#endif

