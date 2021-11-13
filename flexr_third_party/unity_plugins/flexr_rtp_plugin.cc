#include <bits/stdc++.h>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgproc.hpp>

#include <flexr_core/include/types/types.h>
#include <flexr_core/include/types/frame.h>
#include <flexr_core/include/utils/deserialize_functions.h>
#include <flexr_core/include/utils/serialize_functions.h>
#include <flexr_core/include/components/rtp_port.h>

#define DllExport __attribute__((visibility("default")))
#define MAX_SESSION 20


struct UnityColor32
{
	uchar red, green, blue, alpha;
};


class RtpSession
{
  public:
    uvg_rtp::session *session;
    uvg_rtp::media_stream *stream;
    bool isSet;
    uint32_t seq;

    RtpSession(): session(nullptr), stream(nullptr), isSet(false), seq(0)
    {}


    void unrefFrameExceptData(uvg_rtp::frame::rtp_frame *rtpFrame)
    {
      if (rtpFrame->csrc)      delete rtpFrame->csrc;
      if (rtpFrame->ext)       delete rtpFrame->ext;
      if (rtpFrame->probation) delete rtpFrame->probation;
    }


    bool setSession(uvg_rtp::context* ctx, char* addr, int recvPort, int sendPort)
    {
      session = ctx->create_session(addr);
      if(session == nullptr) return false;

      unsigned flags = RCE_NO_SYSTEM_CALL_CLUSTERING | RCE_FRAGMENT_GENERIC;
      stream = session->create_stream(recvPort, sendPort, RTP_FORMAT_GENERIC, flags);
      if(stream == nullptr) return false;

      stream->configure_ctx(RCC_UDP_SND_BUF_SIZE, 20 * 1000 * 1000);
      stream->configure_ctx(RCC_UDP_RCV_BUF_SIZE, 20 * 1000 * 1000);
      stream->configure_ctx(RCC_PKT_MAX_DELAY,    10);
      isSet = true;

      return true;
    }


    void close(uvg_rtp::context* ctx)
    {
      if(stream  != nullptr) session->destroy_stream(stream);
      if(session != nullptr) ctx->destroy_session(session);

      stream  = nullptr;
      session = nullptr;
      isSet   = false;
    }


    bool receiveMsg(bool inBlock, uint8_t* &outBuffer, uint32_t &outSize)
    {
      uvg_rtp::frame::rtp_frame *rtpFrame = nullptr;
      if(outBuffer != nullptr) { delete outBuffer; outBuffer = nullptr; }

      if(inBlock) rtpFrame = stream->pull_frame();
      else        rtpFrame = stream->pull_frame(1);

      if(rtpFrame != nullptr) {
        outSize = rtpFrame->payload_len;
        outBuffer = std::move(rtpFrame->payload);
        unrefFrameExceptData(rtpFrame);
        return true;
      }
      else
        return false;
    }


    bool sendMsg(uint8_t *inData, uint32_t inDataSize, uint32_t ts)
    {
      if (stream->push_frame(inData, inDataSize, ts, RTP_NO_FLAGS) != RTP_OK) return false;
      return true;
    }
};


uvg_rtp::context context;
int numOfSess = 0;
RtpSession sessions[MAX_SESSION];


extern "C" {

  DllExport int RtpAddSession(char* addr, int recvPort, int sendPort)
  {
    if(numOfSess < MAX_SESSION)
    {
      bool res = sessions[numOfSess].setSession(&context, addr, recvPort, sendPort);
      if(res == false) return -1;
      else             return numOfSess++;
    }

    return -1;
  }


  DllExport void RtpClose()
  {
    for(int i = 0; i < MAX_SESSION; i++)
    {
      if(sessions[i].isSet)
      {
        sessions[i].close(&context);
      }
    }
    numOfSess = 0;
  }


  DllExport int RtpGetFrame(int sessionIdx, bool isBlocking, int width, int height, UnityColor32 **unityFrameBuffer)
  {
    if(sessions[sessionIdx].isSet == false) return -1;

    uint8_t* recvBuffer = nullptr;
    uint32_t recvSize   = 0;
    if( sessions[sessionIdx].receiveMsg(isBlocking, recvBuffer, recvSize) )
    {
      // header
      flexr::types::Message<flexr::types::Frame> frame;
      uint32_t msgMetaSize = sizeof(frame);

      memcpy(&frame, recvBuffer, msgMetaSize);

      // size check
      if(recvSize-msgMetaSize != frame.dataSize || recvSize-msgMetaSize != frame.data.dataSize)
      {
        delete recvBuffer;
        return -2;
      }
      else
      {
        frame.data = flexr::types::Frame(height, width, CV_8UC3);
        memcpy(frame.data.data, recvBuffer+msgMetaSize, frame.dataSize);

        // rgb2rgba
        flip(frame.data.useAsCVMat(), frame.data.useAsCVMat(), 1);
        cv::Mat unityFrame(height, width, CV_8UC4, *unityFrameBuffer);
        cvtColor(frame.data.useAsCVMat(), unityFrame, cv::COLOR_RGB2BGRA);

        delete recvBuffer;
        frame.data.release();
        return true;
      }
    }

    return -3;
  }


  DllExport int RtpGetObjectPose(int sessionIdx, bool isBlocking, flexr::types::ObjectPose *objectPose)
  {
    if(sessions[sessionIdx].isSet == false) return -1;

    uint8_t* recvBuffer = nullptr;
    uint32_t recvSize   = 0;
    if( sessions[sessionIdx].receiveMsg(isBlocking, recvBuffer, recvSize) )
    {
      if(recvSize != sizeof(flexr::types::Message<flexr::types::ObjectPose>)) return -2;

      flexr::types::Message<flexr::types::ObjectPose> *poseMsg = (flexr::types::Message<flexr::types::ObjectPose>*) recvBuffer;
      memcpy(objectPose, &poseMsg->data, poseMsg->dataSize);

      delete recvBuffer;
      return true;
    }

    return -3;
  }


  DllExport int RtpGetKey(int sessionIdx, bool isBlocking, char *key)
  {
    if(sessions[sessionIdx].isSet == false) return -1;

    uint8_t* recvBuffer = nullptr;
    uint32_t recvSize   = 0;
    if( sessions[sessionIdx].receiveMsg(isBlocking, recvBuffer, recvSize) )
    {
      if(recvSize != sizeof(flexr::types::Message<char>)) return -2;

      flexr::types::Message<char> *keyMsg = (flexr::types::Message<char>*) recvBuffer;
      *key = keyMsg->data;

      delete recvBuffer;
      return true;
    }

    return -3;
  }


  DllExport int RtpSendFrame(int sessionIdx, bool isBlocking, int width, int height, UnityColor32 **unityFrameBuffer)
  {
    if(sessions[sessionIdx].isSet == false) return -1;

    uint8_t* sendBuffer = nullptr;
    uint32_t sendSize   = 0;

    flexr::types::Message<flexr::types::Frame> frame;

    cv::Mat temp(height, width, CV_8UC3);
    frame.data = flexr::types::Frame(temp);
    frame.setHeader("unity frame", sessions[sessionIdx].seq++, getTsMs(), frame.data.dataSize);

    cv::Mat unityFrame(height, width, CV_8UC4, *unityFrameBuffer);
    cvtColor(unityFrame, frame.data.useAsCVMat(), cv::COLOR_BGRA2RGB);

    flip(frame.data.useAsCVMat(), frame.data.useAsCVMat(), 1);
    flexr::utils::serializeRawFrame(&frame, sendBuffer, sendSize, true);

    sessions[sessionIdx].sendMsg(sendBuffer, sendSize, frame.ts);
    delete sendBuffer;

    return true;
  }

}

