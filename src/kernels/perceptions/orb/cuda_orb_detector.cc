#ifdef __USE_OPENCV_CUDA__
#include <kernels/perceptions/orb/cuda_orb_detector.h>
#include <opencv2/core.hpp>

namespace mxre
{
  namespace kernels
  {
    CudaORBDetector::CudaORBDetector(std::vector<mxre::cv_types::ObjectInfo> registeredObjVec) :
      MXREKernel(), objInfoVec(registeredObjVec)
    {
      addInputPort<mxre::types::Frame>("in_frame");
      addOutputPort<std::vector<mxre::cv_types::ObjectInfo>>("out_obj_info");

      // Object Detection Parameters
      knnMatchRatio = 0.8f;
      knnParam = 5;
      ransacThresh = 2.5f;
      minInlierThresh = 20;

      detector = cv::cuda::ORB::create();
      matcher = cv::cuda::DescriptorMatcher::createBFMatcher(cv::NORM_HAMMING);
    }


    bool CudaORBDetector::logic(mxre::types::Frame *inFrame, std::vector<mxre::cv_types::ObjectInfo> *outObjInfo)
    {
      std::vector<cv::KeyPoint> frameKps;

      // 1. prepare gary frame
      cv::Mat grayFrame = inFrame->useAsCVMat();
      cv::cvtColor(grayFrame, grayFrame, CV_BGR2GRAY);
      cuFrame.upload(grayFrame);

      // 2. run CUDA ORB & convert the GPU result into CPU; cpu kps & gpu desc are ready
      detector->detectAndComputeAsync(cuFrame, cv::noArray(), cuKp, cuDesc, false, stream);
      stream.waitForCompletion();
      detector->convert(cuKp, frameKps);
      inFrame->release(); // deallocate the frame memory

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

        cv::Mat homography;
        cv::Mat inlierMask;
        std::vector<cv::KeyPoint> objInlier, frameInlier;
        std::vector<cv::DMatch> inlierMatches;

        // 4. Find the homography with matched kps (at least 4kps for planar obj)
        if(objMatch.size() > 4) {
          homography = findHomography(mxre::cv_utils::convertKpsToPts(objMatch),
                                      mxre::cv_utils::convertKpsToPts(frameMatch),
                                      cv::RANSAC, ransacThresh, inlierMask);
        }

        // 5. handle the detected object
        if(objMatch.size()>4 && !homography.empty()) {
          for (unsigned i = 0; i < objMatch.size(); i++)
          {
            if (inlierMask.at<uchar>(i))
            {
              int new_i = static_cast<int>(objInlier.size());
              objInlier.push_back(objMatch[i]);
              frameInlier.push_back(frameMatch[i]);
              inlierMatches.push_back(cv::DMatch(new_i, new_i, 0));
            }
          }

          // 5. Draw the object rectangle in the frame via homography and inliers
          if (objInlier.size() >= minInlierThresh)
          {
            objIter->isDetected = true;
            perspectiveTransform(objIter->rect2D, objIter->location2D, homography);
            //mxre::cv_utils::drawBoundingBox(frame.cvMat, objIter->location2D);
          }
          else
            objIter->isDetected = false;
        }
        else objIter->isDetected = false;
      }

      *outObjInfo = objInfoVec;
      return true;
    }


    raft::kstatus CudaORBDetector::run() {
#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif
      debug_print("START");

      auto &inFrame( input["in_frame"].peek<mxre::types::Frame>() );
      auto &outObjInfo( output["out_obj_info"].allocate<std::vector<mxre::cv_types::ObjectInfo>>() );

      if(logic(&inFrame, &outObjInfo)) {
        output["out_obj_info"].send();
        sendPrimitiveCopy("out_obj_info", &outObjInfo);
      }

      recyclePort("in_frame");
      debug_print("END");

#ifdef __PROFILE__
      mxre::types::TimeVal end = getNow();
      profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

      return raft::proceed;
    }

  } // namespace ctx_understanding
} // namespace mxre

#endif

