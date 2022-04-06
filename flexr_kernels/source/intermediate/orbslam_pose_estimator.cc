#ifdef __FLEXR_KERNEL_ORBSLAM_POSE_ESTIMATOR__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>


namespace flexr
{
  namespace kernels
  {

    OrbSlamPoseEstimator::OrbSlamPoseEstimator(std::string id, std::string vocaPath, std::string slamConfigPath,
                                               std::string sensor, bool viewer): FleXRKernel(id), vocaPath(vocaPath),
                                                 slamConfigPath(slamConfigPath), viewer(viewer)
    {
      setName("OrbSlamPoseEstimator");

      // MONOCULAR=0,
      // STEREO=1,
      // RGBD=2,
      // IMU_MONOCULAR=3,
      // IMU_STEREO=4

      if(sensor == "mono")
      {
        portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);
        sensorType = ORB_SLAM3::System::MONOCULAR;
      }
      if(sensor == "mono_inertial")
      {
        portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);
        portManager.registerInPortTag("in_imus",  components::PortDependency::BLOCKING, utils::deserializeDefault<OrbSlamPoseEstimatorImusMsgType>);
        sensorType = ORB_SLAM3::System::IMU_MONOCULAR;
      }
      if(sensor == "stereo")
      {
        // TODO
        sensorType = ORB_SLAM3::System::STEREO;
      }
      if(sensor == "stereo_inertial")
      {
        // TODO
        sensorType = ORB_SLAM3::System::IMU_STEREO;
      }
      if(sensor == "rgbd")
      {
        // TODO
        sensorType = ORB_SLAM3::System::RGBD;
      }

      isInit = false;
      portManager.registerOutPortTag("out_pose", flexr::utils::sendLocalFrameCopy, flexr::utils::serializeRawFrame);
    }


    OrbSlamPoseEstimator::~OrbSlamPoseEstimator()
    {
      SLAM->Shutdown();
      delete SLAM;
    }


    raft::kstatus OrbSlamPoseEstimator::run()
    {
      if(isInit == false)
      {
        isInit = true;
        SLAM = new ORB_SLAM3::System(vocaPath, slamConfigPath, sensorType, viewer);
      }
      OrbSlamPoseEstimatorImgMsgType  *inFrame = portManager.getInput<OrbSlamPoseEstimatorImgMsgType>("in_frame");
      OrbSlamPoseEstimatorImusMsgType *inImus  = portManager.getInput<OrbSlamPoseEstimatorImusMsgType>("in_imus");
      OrbSlamPoseEstimatorPoseMsgType *outPose = portManager.getOutputPlaceholder<OrbSlamPoseEstimatorPoseMsgType>("out_pose");

      double st = getTsNow();
      std::vector<ORB_SLAM3::IMU::Point> imus;
      for(int i = 0; i < inImus->data.size(); i++)
        imus.push_back(ORB_SLAM3::IMU::Point(
              inImus->data[i].a.x,
              inImus->data[i].a.y,
              inImus->data[i].a.z,
              inImus->data[i].g.x,
              inImus->data[i].g.y,
              inImus->data[i].g.z,
              inImus->data[i].t));

      //cv::Mat img = inFrame->data.useAsCVMat().clone();

      cv::Mat curPose = SLAM->TrackMonocular(inFrame->data.useAsCVMat(), inFrame->ts, imus);
      if(curPose.empty() == false)
      {
        outPose->data = flexr::types::Frame(curPose);
        outPose->setHeader("OrbCamPose", inFrame->seq, inFrame->ts, curPose.total()*curPose.elemSize());

        double et = getTsNow();
        if(logger.isSet()) logger.getInstance()->info("Pose Estimation\t start\t{}\t end\t{}\t exe\t{}", st, et, et-st);
        //debug_print("Pose Estimation Exe: %f", et-st);
        //debug_print("Mat Size: %d / %d", outPose->dataSize, outPose->data.elemSize * outPose->data.totalElem);

        portManager.sendOutput<OrbSlamPoseEstimatorPoseMsgType>("out_pose", outPose);
      }

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);
      portManager.freeInput("in_imus", inImus);
      freqManager.adjust();

      return raft::proceed;
    }


  } // namespace kernels
} // namespace flexr

#endif

