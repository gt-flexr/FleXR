#ifndef __FLEXR_RTP_FRAME_SENDER__
#define __FLEXR_RTP_FRAME_SENDER__

#include <bits/stdc++.h>
#include <raft>

#include "defs.h"
#include "kernels/kernel.h"
#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"
#include "components/rtp_sender.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/frame.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

namespace flexr
{
  namespace kernels
  {

    using FrameSenderMsgType = types::Message<types::Frame>;


    /**
     * @brief Kernel to send RTP frame stream
     *
     * Port Tag       | Type
     * ---------------| ----------------------------
     * in_frame       | @ref flexr::types::Message<@ref flexr::types::Frame>
     */
    class RTPFrameSender : public FleXRKernel
    {
      private:
        components::RTPSender rtpSender;

        std::string encoderName;
        int width, height;

        AVCodec* encoder;
        AVCodecContext *encoderContext;
        AVFrame *encodingFrame;

        cv::Mat yuvFrame;

      public:
        /**
         * @brief Initialize RTP frame sender
         * @param adder
         *  IP address of remote node to send the stream
         * @param port
         *  Port number of remote node to send the stream
         * @param encoderName
         *  Encoder name to encode sending frames
         * @param width
         *  Frame width
         * @param height
         *  Frame height
         * @param fps
         *  Target FPS of encoding w.r.t. bitrates
         * @see flexr::components::RTPSender
         */
        RTPFrameSender(std::string addr, int port, std::string encoderName, int width, int height,
                       int bitrate, int fps=60);


        ~RTPFrameSender();


        raft::kstatus run() override;


        void activateInPortAsRemote(const std::string tag, int portNumber)
        {
          debug_print("not allow remote port activation.");
        }
    };

  } // namespace kernels
} // namespace flexr

#endif

