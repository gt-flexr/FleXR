#include <kernels/intermediate/aruco_cam_locator.h>
#include <utils/msg_sending_functions.h>

namespace flexr
{
  namespace kernels
  {

    /* Constructor */
    ArUcoCamLocator::ArUcoCamLocator(std::string id, cv::aruco::PREDEFINED_DICTIONARY_NAME dictName,
                                     int width, int height): FleXRKernel(id)
    {
      setName("ArUcoCamLocator");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, 0);
      portManager.registerOutPortTag("out_cam_pose",
                                     utils::sendLocalBasicCopy<ArUcoCamLocatorOutPoseType>,
                                     utils::sendRemotePrimitive<ArUcoCamLocatorOutPoseType>,
                                     types::freePrimitiveMsg<ArUcoCamLocatorOutPoseType>);
      markerDict = cv::aruco::getPredefinedDictionary(dictName);

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


    raft::kstatus ArUcoCamLocator::run()
    {
      ArUcoCamLocatorInFrameType *inFrame = portManager.getInput<ArUcoCamLocatorInFrameType>("in_frame");
      ArUcoCamLocatorOutPoseType *outCamPose = \
                                          portManager.getOutputPlaceholder<ArUcoCamLocatorOutPoseType>("out_cam_pose");

      std::vector<int> ids;
      std::vector<std::vector<cv::Point2f>> corners;

      double st, et;

      st = getTsMs();
      cv::aruco::detectMarkers(inFrame->data.useAsCVMat(), markerDict, corners, ids);
      if(ids.size() > 0)
      {
        std::vector<cv::Vec3d> rvecs, tvecs;
        cv::aruco::estimatePoseSingleMarkers(corners, 0.05, camIntrinsic, camDistCoeffs, rvecs, tvecs);

        cv::Mat Rt, R;
        cv::Rodrigues(rvecs[0], Rt);
        cv::transpose(Rt, R);

        double roll  = atan2(R.at<double>(2, 1), R.at<double>(2, 2)) * (180/PI);
        double pitch = asin(-R.at<double>(2, 0)) * (180/PI);
        double yaw   = atan2(-R.at<double>(1, 0), R.at<double>(0, 0)) * (180/PI);

        cv::Mat tvec = -R * tvecs[0];
        debug_print("Cam Rotation: %f / %f / %f", roll, pitch, yaw);
        debug_print("Cam Translation: %f / %f / %f", tvec.at<double>(0), tvec.at<double>(1), tvec.at<double>(2));

        outCamPose->data.rx = roll;
        outCamPose->data.ry = pitch;
        outCamPose->data.rz = yaw;

        outCamPose->data.tx = tvec.at<double>(0);
        outCamPose->data.ty = tvec.at<double>(1);
        outCamPose->data.tz = tvec.at<double>(2);
      }
      et = getTsMs();

      portManager.sendOutput("out_cam_pose", outCamPose);
      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);

      debug_print("st(%lf) et(%lf) exe(%lf)", st, et, et-st);
      if(logger.isSet()) logger.getInstance()->info("{}\t {}\t {}", st, et, et-st);
      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

