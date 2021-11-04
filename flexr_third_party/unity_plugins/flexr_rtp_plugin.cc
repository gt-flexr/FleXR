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

flexr::components::RtpPort rtpPort;
bool isSet = false;
uint32_t seq = 0;


struct UnityColor32
{
	uchar red, green, blue, alpha;
};


extern "C" {

  DllExport bool RtpInit(char* addr, int recvPort, int sendPort) {
    std::string strAddr(addr);
    bool ret = rtpPort.init(strAddr, recvPort, sendPort);

    if(ret == true)
    {
      isSet = true;
      return true;
    }
    else
    {
      return false;
    }
  }


  DllExport bool RtpReceiveFrame(bool isBlocking, int width, int height, UnityColor32 **unityFrameBuffer) {
    if(isSet == false)
    {
      return false;
    }

    uint8_t* recvBuffer = nullptr;
    uint32_t recvSize, ts;

    if( rtpPort.receiveMsg(isBlocking, recvBuffer, recvSize, ts) )
    {
      flexr::types::Message<flexr::types::Frame> frameMsg;
      frameMsg.data = flexr::types::Frame(height, width, CV_8UC3);
      flexr::types::Message<flexr::types::Frame> *frameMsgPointer = &frameMsg;

      flexr::utils::deserializeRawFrame(recvBuffer, recvSize, (void**)&frameMsgPointer);

      flip(frameMsg.data.useAsCVMat(), frameMsg.data.useAsCVMat(), 0);

      cv::Mat unityFrame(height, width, CV_8UC4, *unityFrameBuffer);
      cvtColor(frameMsg.data.useAsCVMat(), unityFrame, cv::COLOR_RGB2BGRA);

      frameMsg.data.release();
      return true;
    }

    return false;
  }


  DllExport bool RtpReceiveObjectPose(bool isBlocking, flexr::types::ObjectPose *objectPose)
  {
    if(isSet == false)
    {
      return false;
    }

    uint8_t* recvBuffer = nullptr;
    uint32_t recvSize, ts;
    if( rtpPort.receiveMsg(isBlocking, recvBuffer, recvSize, ts) )
    {
      flexr::types::Message<flexr::types::ObjectPose> *poseMsg = (flexr::types::Message<flexr::types::ObjectPose>*) recvBuffer;
      *objectPose = poseMsg->data;

      delete recvBuffer;
      return true;
    }

    return false;
  }


  DllExport bool RtpReceiveKey(bool isBlocking, char *key)
  {
    if(isSet == false)
    {
      return false;
    }

    uint8_t* recvBuffer = nullptr;
    uint32_t recvSize, ts;
    if( rtpPort.receiveMsg(isBlocking, recvBuffer, recvSize, ts) )
    {
      flexr::types::Message<char> *keyMsg = (flexr::types::Message<char>*) recvBuffer;
      *key = keyMsg->data;

      delete recvBuffer;
      return true;
    }

    return false;
  }


  DllExport bool RtpSendFrame(bool isBlocking, int width, int height, UnityColor32 **unityFrameBuffer) {
    if(isSet == false)
    {
      return false;
    }

    uint8_t* sendBuffer = nullptr;
    uint32_t sendSize, ts;

    flexr::types::Message<flexr::types::Frame> frameMsg;

    cv::Mat temp(height, width, CV_8UC3);
    frameMsg.data = flexr::types::Frame(temp);
    frameMsg.setHeader("unity frame", seq++, getTsMs(), frameMsg.data.dataSize);

    cv::Mat unityFrame(height, width, CV_8UC4, *unityFrameBuffer);
    cvtColor(unityFrame, frameMsg.data.useAsCVMat(), cv::COLOR_BGRA2RGB);

    flip(frameMsg.data.useAsCVMat(), frameMsg.data.useAsCVMat(), 0);
    flexr::utils::serializeRawFrame(&frameMsg, sendBuffer, sendSize);

    rtpPort.send(sendBuffer, sendSize, frameMsg.ts);
    delete sendBuffer;

    return true;
  }

}

