#include <kernels/intermediate/aruco_detector.h>
#include <utils/msg_sending_functions.h>

namespace flexr
{
  namespace kernels
  {

    /* Constructor */
    ArUcoDetector::ArUcoDetector(std::string id, cv::aruco::PREDEFINED_DICTIONARY_NAME dictName,
        int width, int height): FleXRKernel(id)
    {
      setName("ArUcoDetector");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, 0);
      portManager.registerOutPortTag("out_marker_poses",
                                     utils::sendLocalBasicCopy<ArUcoDetectorOutPosesType>,
                                     utils::sendRemotePrimitiveVecData<ArUcoDetectorOutPosesType>,
                                     types::freePrimitiveMsg<ArUcoDetectorOutPosesType>);
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
      ArUcoDetectorOutPosesType *outMarkerPoses = portManager.getOutputPlaceholder<ArUcoDetectorOutPosesType>("out_marker_poses");

      std::vector<int> ids;
      std::vector<std::vector<cv::Point2f>> corners;

      double st, et;

      st = getTsMs();
      cv::aruco::detectMarkers(inFrame->data.useAsCVMat(), markerDict, corners, ids);
      if(ids.size() > 0)
      {
        std::vector<cv::Vec3d> rvecs, tvecs;
        cv::aruco::estimatePoseSingleMarkers(corners, 0.05, camIntrinsic, camDistCoeffs, rvecs, tvecs);

        outMarkerPoses->data.resize(MAX_MARKERS);
        for(int i = 0; i < MAX_MARKERS; i++)
        {
          cv::Mat R;
          cv::Rodrigues(rvecs[i], R);
          double roll  = atan2(R.at<double>(2, 1), R.at<double>(2, 2)) * (180/PI);
          double pitch = asin(R.at<double>(2, 0)) * (180/PI);
          double yaw   = atan2(-R.at<double>(1, 0), R.at<double>(0, 0)) * (180/PI);

          if(i == 0)
          {
            debug_print("Marker Rotation: %f / %f / %f", roll, pitch, yaw);
            /*
            cv::Mat mtxR, mtxQ, Qx, Qy, Qz;
            cv::RQDecomp3x3(Rt, mtxR, mtxQ, Qx, Qy, Qz);
            std::cout << Qx << std::endl;
            std::cout << Qy << std::endl;
            std::cout << Qz << std::endl << std::endl;
            */
          }

          outMarkerPoses->data[i].rx = roll;
          outMarkerPoses->data[i].ry = pitch;
          outMarkerPoses->data[i].rz = yaw;

          outMarkerPoses->data[i].tx = tvecs[i][0];
          outMarkerPoses->data[i].ty = tvecs[i][1];
          outMarkerPoses->data[i].tz = tvecs[i][2];
        }
      }
      et = getTsMs();

      portManager.sendOutput("out_marker_poses", outMarkerPoses);
      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);

      debug_print("st(%lf) et(%lf) exe(%lf)", st, et, et-st);
      if(logger.isSet()) logger.getInstance()->info("{}\t {}\t {}", st, et, et-st);
      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

