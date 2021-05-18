#include <kernels/intermediate/orb_detector.h>
#include <utils/msg_sending_functions.h>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    /* Constructor */
    ORBDetector::ORBDetector(std::vector<mxre::cv_types::MarkerInfo> registeredMarkers):
      MXREKernel(), registeredMarkers(registeredMarkers)
    {
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, 0);
      portManager.registerOutPortTag("out_detected_markers",
                                     utils::sendLocalBasicCopy<ORBDetectorOutMarkerType>,
                                     utils::sendRemoteMarkers,
                                     types::freePrimitiveMsg<ORBDetectorOutMarkerType>);

      // Object Detection Parameters
      knnMatchRatio = 0.9f;
      knnParam = 3;
      ransacThresh = 3.0f;
      minInlierThresh = 20;

      detector = cv::ORB::create();
      matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
    }

    bool ORBDetector::logic(ORBDetectorInFrameType *inFrame, ORBDetectorOutMarkerType *outDetectedMarkers)
    {
      std::vector<cv::KeyPoint> frameKps;
      cv::Mat frameDesc;
      strcpy(outDetectedMarkers->tag, inFrame->tag);
      outDetectedMarkers->seq = inFrame->seq;
      outDetectedMarkers->ts  = inFrame->ts;

      // 0. prepare gary frame
      cv::Mat grayFrame = inFrame->data.useAsCVMat();
      cv::cvtColor(grayFrame, grayFrame, CV_BGR2GRAY);

      // 1. figure out frame keypoints and descriptors to detect objects in the frame
      detector->detectAndCompute(grayFrame, cv::noArray(), frameKps, frameDesc);

      // multiple object detection
      std::vector<mxre::cv_types::MarkerInfo>::iterator markerInfo;
      for (markerInfo = registeredMarkers.begin(); markerInfo != registeredMarkers.end(); ++markerInfo)
      {
        // 2. use the matcher to find correspondence
        std::vector<std::vector<cv::DMatch>> matches;
        std::vector<cv::KeyPoint> objMatch, frameMatch;
        cv::Mat homography;
        cv::Mat inlierMask;
        std::vector<cv::KeyPoint> objInlier, frameInlier;
        std::vector<cv::DMatch> inlierMatches;

        // 2.1. get all the matches between object and frame kps based on desc
        matcher->knnMatch(markerInfo->desc, frameDesc, matches, knnParam);

        // 2.2. add close-enough matches by distance (filtered matches)
        for (unsigned i = 0; i < matches.size(); i++)
        {
          if (matches[i][0].distance < knnMatchRatio * matches[i][1].distance) // 1st and 2nd diff distance check
          {
            objMatch.push_back(markerInfo->kps[matches[i][0].queryIdx]);
            frameMatch.push_back(frameKps[matches[i][0].trainIdx]);
          }
        }

        // 3. get the homography from the matches
        if (objMatch.size() >= 4)
        {
          homography = findHomography(mxre::cv_utils::convertKpsToPts(objMatch),
                                      mxre::cv_utils::convertKpsToPts(frameMatch),
                                      cv::RANSAC, ransacThresh, inlierMask);
        }

        // 4. if there is an object in the frame, check the inlier points and save matched inlier points
        if (objMatch.size() >= 4 && !homography.empty())
        {
          // 5. find the inliers among the matched keypoints
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
            //mxre::cv_utils::drawBoundingBox(frame.cvMat, objIter->location2D);
          }
        }
      }

      return true;
    }

    raft::kstatus ORBDetector::run()
    {
      ORBDetectorInFrameType *inFrame = portManager.getInput<ORBDetectorInFrameType>("in_frame");
      ORBDetectorOutMarkerType *outDetectedMarkers = \
                                    portManager.getOutputPlaceholder<ORBDetectorOutMarkerType>("out_detected_markers");

      double st = getTsNow();
      logic(inFrame, outDetectedMarkers);
      portManager.sendOutput("out_detected_markers", outDetectedMarkers);
      double et = getTsNow();

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);


      if(debugMode) debug_print("st(%lf) et(%lf) exe(%lf)", st, et, et-st);
      if(logger.isSet()) logger.getInstance()->info("{}\t {}\t {}", st, et, et-st);
      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

