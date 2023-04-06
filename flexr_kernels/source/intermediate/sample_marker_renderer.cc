#ifdef __FLEXR_KERNEL_SAMPLE_MARKER_RENDERER__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {

    /* Constructor */
    SampleMarkerRenderer::SampleMarkerRenderer(std::string id, int width, int height): FleXRKernel(id)
    {
      setName("SampleMarkerRenderer");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);
      portManager.registerInPortTag("in_cam_pose", components::PortDependency::NONBLOCKING);
      portManager.registerInPortTag("in_key", components::PortDependency::NONBLOCKING);
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy, utils::serializeRawFrame);

      this->width = width; this->height = height;
      camIntrinsic  = cv::Mat(3, 3, CV_64FC1);
      camIntrinsic.at<double>(0, 0) = width;
      camIntrinsic.at<double>(0, 1) = 0;
      camIntrinsic.at<double>(0, 2) = width/2;
      camIntrinsic.at<double>(1, 0) = 0;
      camIntrinsic.at<double>(1, 1) = width;
      camIntrinsic.at<double>(1, 2) = height/2;
      camIntrinsic.at<double>(2, 0) = 0;
      camIntrinsic.at<double>(2, 1) = 0;
      camIntrinsic.at<double>(2, 2) = 1;
      camDistCoeffs = cv::Mat(4, 1, CV_64FC1, {0, 0, 0, 0});
    }


    raft::kstatus SampleMarkerRenderer::run()
    {
      double st, et;
      SamMarRendFrame     *inFrame   = portManager.getInput<SamMarRendFrame>("in_frame");
      SamMarRendInCamPose *inCamPose = portManager.getInput<SamMarRendInCamPose>("in_cam_pose");
      SamMarRendInKey     *inKey     = portManager.getInput<SamMarRendInKey>("in_key");

      if(inKey != nullptr)
        if(inKey->data != 0) debug_print("input key: %c", inKey->data);

      if(inFrame != nullptr)
      {
        if (inFrame->data.useAsCVMat().empty() == false)
        {

          SamMarRendFrame *outFrame = portManager.getOutputPlaceholder<SamMarRendFrame>("out_frame");

          st = getTsMs();
          outFrame->setHeader(inFrame->tag, inFrame->seq, inFrame->ts, inFrame->dataSize);
          outFrame->data = inFrame->data;

          cv::Vec3d rvec, tvec;
          if (inCamPose != nullptr)
          {
            rvec = cv::Vec3d(inCamPose->data.rx, inCamPose->data.ry, inCamPose->data.rz);
            tvec = cv::Vec3d(inCamPose->data.tx, inCamPose->data.ty, inCamPose->data.tz);
            debug_print("Marker Rotation: %f / %f / %f", inCamPose->data.rx, inCamPose->data.ry, inCamPose->data.rz);
            cv::aruco::drawAxis(outFrame->data.useAsCVMat(), camIntrinsic, camDistCoeffs, rvec, tvec, 0.05);
          }

          et = getTsMs();

          portManager.sendOutput("out_frame", outFrame);
          if (logger.isSet())
            logger.getInstance()->info("{}\t {}\t {}", st, et, et - st);
        }
      }
      else
      {
        debug_print("received corrupted frame");
      }

      portManager.freeInput("in_frame", inFrame);
      portManager.freeInput("in_key", inKey);
      portManager.freeInput("in_cam_pose", inCamPose);

      debug_print("st(%lf) et(%lf) exe(%lf)", st, et, et-st);

      freqManager.adjust();
      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

#endif

