#ifdef __USE_OPENCV_CUDA__

#include <unistd.h>
#include <opencv2/core.hpp>
#include <kernels/intermediate/cuda_orb_detector.h>
#include <types/types.h>


namespace flexr
{
  namespace kernels
  {
    CudaORBDetector::CudaORBDetector(std::string id, std::string markerImage): FleXRKernel(id)
    {
      setName("CudaORBDetector");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING);
      portManager.registerOutPortTag("out_detected_markers",
                                     utils::sendLocalBasicCopy<CudaORBDetectorOutMarkerType>);

      orbMarkerTracker.setMarkerFromImage(markerImage);
      registeredMarkers = orbMarkerTracker.getRegisteredObjects();

      // Object Detection Parameters
      knnMatchRatio = 0.9f;
      knnParam = 3;
      ransacThresh = 3.0f;
      minInlierThresh = 20;

      numKps = 0;

      detector = cv::cuda::ORB::create();
      matcher = cv::cuda::DescriptorMatcher::createBFMatcher(cv::NORM_HAMMING);
    }


    bool CudaORBDetector::logic(CudaORBDetectorInFrameType *inFrame, CudaORBDetectorOutMarkerType *outDetectedMarkers)
    {
      std::vector<cv::KeyPoint> frameKps;
      strcpy(outDetectedMarkers->tag, inFrame->tag);
      outDetectedMarkers->seq = inFrame->seq;
      outDetectedMarkers->ts  = inFrame->ts;

      // 1. prepare gary frame
      cv::Mat grayFrame = inFrame->data.useAsCVMat();
      cv::cvtColor(grayFrame, grayFrame, cv::COLOR_RGB2GRAY);
      cuFrame.upload(grayFrame);

      // 2. run CUDA ORB & convert the GPU result into CPU; cpu kps & gpu desc are ready
      detector->detectAndComputeAsync(cuFrame, cv::noArray(), cuKp, cuDesc, false, stream);
      stream.waitForCompletion();
      detector->convert(cuKp, frameKps);
      numKps = frameKps.size();

      // 3. multi-obj detection
      std::vector<flexr::cv_types::MarkerInfo>::iterator markerInfo;
      for(markerInfo = registeredMarkers.begin(); markerInfo!=registeredMarkers.end(); ++markerInfo){
        cv::cuda::GpuMat cuObjDesc;
        std::vector<std::vector<cv::DMatch>> matches;
        std::vector<cv::KeyPoint> objMatch, frameMatch;
        cv::Mat homography;
        cv::Mat inlierMask;
        std::vector<cv::KeyPoint> objInlier, frameInlier;
        std::vector<cv::DMatch> inlierMatches;

        // 3-1. upload each objDesc into cuDesc
        cuObjDesc.upload(markerInfo->desc);

        // 3-2. find matched descs for finding matching kps
        matcher->knnMatchAsync(cuObjDesc, cuDesc, cuMatches, knnParam, cv::noArray(), stream);
        stream.waitForCompletion();
        matcher->knnMatchConvert(cuMatches, matches);

        // 3-3. with matched descs, find corresponding keypoints
        for(unsigned i = 0; i < matches.size(); i++) {
          if(matches[i][0].distance < knnMatchRatio * matches[i][1].distance) {
            objMatch.push_back(markerInfo->kps[matches[i][0].queryIdx]);
            frameMatch.push_back(frameKps[matches[i][0].trainIdx]);
          }
        }

        // 4. Find the homography with matched kps (at least 4kps for planar obj)
        if(objMatch.size() > 4) {
          homography = findHomography(flexr::cv_utils::convertKpsToPts(objMatch),
                                      flexr::cv_utils::convertKpsToPts(frameMatch),
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
            cv_types::DetectedMarker detectedMarker;
            detectedMarker.index = markerInfo->index;
            detectedMarker.defaultLocationIn3D = markerInfo->defaultLocationIn3D;
            perspectiveTransform(markerInfo->defaultLocationIn2D, detectedMarker.locationIn2D, homography);
            outDetectedMarkers->data.push_back(detectedMarker);
          }
        }
      }

      return true;
    }


    raft::kstatus CudaORBDetector::run() {
      CudaORBDetectorInFrameType *inFrame = portManager.getInput<CudaORBDetectorInFrameType>("in_frame");
      CudaORBDetectorOutMarkerType *outDetectedMarkers = \
                                portManager.getOutputPlaceholder<CudaORBDetectorOutMarkerType>("out_detected_markers");

      double st = getTsNow();
      logic(inFrame, outDetectedMarkers);
      portManager.sendOutput("out_detected_markers", outDetectedMarkers);
      double et = getTsNow();

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);


      debug_print("st(%lf) et(%lf) exe(%lf)", st, et, et-st);
      if(logger.isSet()) logger.getInstance()->info("{}\t {}\t {}\t {}", st, et, et-st, numKps);
      numKps = 0;

      return raft::proceed;
    }

  } // namespace ctx_understanding
} // namespace flexr

#endif

