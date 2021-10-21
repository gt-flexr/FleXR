#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {

    /* Constructor */
    ArUcoDetector::ArUcoDetector(std::string id, cv::aruco::PREDEFINED_DICTIONARY_NAME dictName,
        int width, int height): FleXRKernel(id)
    {
      setName("ArUcoDetector");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);
      portManager.registerOutPortTag("out_marker_pose",
                                     utils::sendLocalBasicCopy<ArUcoDetectorOutPoseType>);
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


    raft::kstatus ArUcoDetector::run()
    {
      ArUcoDetectorInFrameType *inFrame = portManager.getInput<ArUcoDetectorInFrameType>("in_frame");
      ArUcoDetectorOutPoseType *outMarkerPose = portManager.getOutputPlaceholder<ArUcoDetectorOutPoseType>("out_marker_pose");

      std::vector<int> ids;
      std::vector<std::vector<cv::Point2f>> corners;

      double st, et;

      st = getTsMs();
      cv::aruco::detectMarkers(inFrame->data.useAsCVMat(), markerDict, corners, ids);
      if(ids.size() > 0)
      {
        std::vector<cv::Vec3d> rvecs, tvecs;
        cv::aruco::estimatePoseSingleMarkers(corners, 0.05, camIntrinsic, camDistCoeffs, rvecs, tvecs);

        cv::Mat R;
        cv::Rodrigues(rvecs[0], R);

        double roll  = atan2(R.at<double>(2, 1), R.at<double>(2, 2)) * (180/PI);
        double pitch = asin(R.at<double>(2, 0)) * (180/PI);
        double yaw   = atan2(-R.at<double>(1, 0), R.at<double>(0, 0)) * (180/PI);

        debug_print("Marker Rotation: %f / %f / %f", roll, pitch, yaw);

        outMarkerPose->data.rx = roll;
        outMarkerPose->data.ry = pitch;
        outMarkerPose->data.rz = yaw;

        outMarkerPose->data.tx = tvecs[0][0];
        outMarkerPose->data.ty = tvecs[0][1];
        outMarkerPose->data.tz = tvecs[0][2];
      }
      et = getTsMs();

      portManager.sendOutput("out_marker_pose", outMarkerPose);
      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);

      debug_print("st(%lf) et(%lf) exe(%lf)", st, et, et-st);
      if(logger.isSet()) logger.getInstance()->info("{}\t {}\t {}", st, et, et-st);
      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

