#include <kernels/intermediate/orb_detector.h>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    /* Constructor */
    ORBDetector::ORBDetector(std::vector<mxre::cv_types::MarkerInfo> registeredMarkers):
      MXREKernel(), registeredMarkers(registeredMarkers)
    {
      addInputPort<mxre::types::Frame>("in_frame");
      addOutputPort<std::vector<mxre::cv_types::DetectedMarker>>("out_detected_markers");

      // Object Detection Parameters
      knnMatchRatio = 0.8f;
      knnParam = 5;
      ransacThresh = 2.5f;
      minInlierThresh = 20;

      detector = cv::ORB::create();
      matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("orb_detector", "logs/" + std::to_string(pid) + "/orb_detector.log");
#endif

    }


    /* Destructor */
    ORBDetector::~ORBDetector() {}


    /* Kernel Logic */
    bool ORBDetector::logic(mxre::types::Frame *inFrame,
                            std::vector<mxre::cv_types::DetectedMarker> *outDetectedMarkers) {
      std::vector<cv::KeyPoint> frameKps;
      cv::Mat frameDesc;

      // 0. prepare gary frame
      cv::Mat grayFrame = inFrame->useAsCVMat();
      cv::cvtColor(grayFrame, grayFrame, CV_BGR2GRAY);


      // 1. figure out frame keypoints and descriptors to detect objects in the frame
      detector->detectAndCompute(grayFrame, cv::noArray(), frameKps, frameDesc);
      inFrame->release(); // deallocate the frame memory

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
            outDetectedMarkers->push_back(detectedMarker);
            //mxre::cv_utils::drawBoundingBox(frame.cvMat, objIter->location2D);
          }
        }
      }

      return true;
    }


    /* Kernel Run */
    raft::kstatus ORBDetector::run() {
      auto &inFrame( input["in_frame"].peek<mxre::types::Frame>() );
      auto &outDetectedMarkers(output["out_detected_markers"].allocate<std::vector<mxre::cv_types::DetectedMarker>>());

#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif


      if(logic(&inFrame, &outDetectedMarkers)) {
        output["out_detected_markers"].send();
        sendPrimitiveCopy("out_detected_markers", &outDetectedMarkers);
      }

      recyclePort("in_frame");

#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}\t {}\t {}", startTimeStamp, endTimeStamp, endTimeStamp-startTimeStamp);
#endif

      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

