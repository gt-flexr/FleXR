#ifdef __USE_OPENCV_CUDA__
#include <kernels/perceptions/orb/cuda_orb_detector.h>
#include <opencv2/core.hpp>

namespace mxre
{
  namespace kernels
  {
    CudaORBDetector::CudaORBDetector(std::vector<mxre::cv_types::ObjectInfo> registeredObjVec) :
      objInfoVec(registeredObjVec), raft::kernel()
    {
      input.addPort<mxre::cv_types::Mat>("in_frame");

      output.addPort<mxre::cv_types::Mat>("out_frame");
      output.addPort<std::vector<mxre::cv_types::ObjectInfo>>("out_obj_info");

#ifdef __PROFILE__
      input.addPort<mxre::types::FrameStamp>("frame_stamp");
      output.addPort<mxre::types::FrameStamp>("frame_stamp");
#endif

      // Object Detection Parameters
      knnMatchRatio = 0.8f;
      knnParam = 5;
      ransacThresh = 2.5f;
      minInlierThresh = 10;

      detector = cv::cuda::ORB::create();
      matcher = cv::cuda::DescriptorMatcher::createBFMatcher(cv::NORM_HAMMING);
    }


    CudaORBDetector::~CudaORBDetector() {}


    raft::kstatus CudaORBDetector::run() {
#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif
      auto &frame( input["in_frame"].peek<mxre::cv_types::Mat>() );

      auto &out_frame( output["out_frame"].allocate<mxre::cv_types::Mat>() );
      auto &out_obj_info( output["out_obj_info"].allocate<std::vector<mxre::cv_types::ObjectInfo>>() );

      cv::Mat grayFrame;
      std::vector<cv::KeyPoint> frameKps;

      // 1. prepare gary frame
      cv::cvtColor(frame.cvMat, grayFrame, CV_BGR2GRAY);
      cuFrame.upload(grayFrame);

      // 2. run CUDA ORB & convert the GPU result into CPU; cpu kps & gpu desc are ready
      detector->detectAndComputeAsync(cuFrame, cv::noArray(), cuKp, cuDesc, false, stream);
      stream.waitForCompletion();
      detector->convert(cuKp, frameKps);

      // 3. multi-obj detection
      std::vector<mxre::cv_types::ObjectInfo>::iterator objIter;
      for(objIter = objInfoVec.begin(); objIter!=objInfoVec.end(); ++objIter) {
        cv::cuda::GpuMat cuObjDesc;
        std::vector< std::vector<cv::DMatch> > matches;
        std::vector<cv::KeyPoint> objMatch, frameMatch;

        // 3-1. upload each objDesc into cuDesc
        cuObjDesc.upload(objIter->desc);

        // 3-2. find matched descs for finding matching kps
        matcher->knnMatchAsync(cuObjDesc, cuDesc, cuMatches, 2, cv::noArray(), stream);
        stream.waitForCompletion();
        matcher->knnMatchConvert(cuMatches, matches);

        // 3-3. with matched descs, find corresponding keypoints
        for(unsigned i = 0; i < matches.size(); i++) {
          if(matches[i][0].distance < knnMatchRatio * matches[i][1].distance) {
            objMatch.push_back(objIter->kps[matches[i][0].queryIdx]);
            frameMatch.push_back(frameKps[matches[i][0].trainIdx]);
          }
        }

        cv::Mat inlierMask, homography;
        std::vector<cv::KeyPoint> objKpInlier, frameKpInlier;
        std::vector<cv::DMatch> kpInlierMatch;

        // 4. Find the homography with matched kps (at least 4kps for planar obj)
        if(objMatch.size() > 4) {
          homography = findHomography(mxre::cv_utils::convertKpsToPts(objMatch),
                                      mxre::cv_utils::convertKpsToPts(frameMatch),
                                      cv::RANSAC, ransacThresh, inlierMask);
        }

        // 5. handle the detected object
        if(objMatch.size()>4 && !homography.empty()) {
          objIter->isDetected = true;
          cv::perspectiveTransform(objIter->rect2D, objIter->location2D, homography);
          mxre::cv_utils::drawBoundingBox(frame.cvMat, objIter->location2D);
        }
        else objIter->isDetected = false;
      }

      out_frame = frame;
      out_obj_info = objInfoVec;

      input["in_frame"].recycle();

#ifdef __PROFILE__
      mxre::types::TimeVal end = getNow();
      profile_print("Exe Time: %lfms", getExeTime(end, start));

      auto &inFrameStamp( input["frame_stamp"].peek<mxre::types::FrameStamp>() );
      auto &outFrameStamp( output["frame_stamp"].allocate<mxre::types::FrameStamp>() );
      outFrameStamp = inFrameStamp;
      input["frame_stamp"].recycle();
      output["frame_stamp"].send();
#endif

      output["out_frame"].send();
      output["out_obj_info"].send();
      return raft::proceed;
    }

  } // namespace ctx_understanding
} // namespace mxre

#endif

