#include <kernels/perceptions/orb/orb_detector.h>

namespace mxre
{
  namespace kernels
  {
    ORBDetector::ORBDetector(std::vector<mxre::cv_types::ObjectInfo> registeredObjs,
                                    cv::Ptr<cv::Feature2D> _detector,
                                    cv::Ptr<cv::DescriptorMatcher> _matcher) : objInfos(registeredObjs),
                                                                              detector(_detector),
                                                                              matcher(_matcher), raft::kernel()
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
    }


    ORBDetector::~ORBDetector() {}


    raft::kstatus ORBDetector::run() {

#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif

      std::vector<cv::KeyPoint> frameKps;
      cv::Mat frameDesc;
      auto &frame( input["in_frame"].peek<mxre::cv_types::Mat>() );

      // prepare output for the next kernel
      auto &out_frame( output["out_frame"].allocate<mxre::cv_types::Mat>() );
      auto &out_obj_info( output["out_obj_info"].allocate<std::vector<mxre::cv_types::ObjectInfo>>() );

      // 1. figure out frame keypoints and descriptors to detect objects in the frame
      detector->detectAndCompute(frame.cvMat, cv::noArray(), frameKps, frameDesc);

      // multiple object detection
      std::vector<mxre::cv_types::ObjectInfo>::iterator objIter;
      for (objIter = objInfos.begin(); objIter != objInfos.end(); ++objIter)
      {
        // 2. use the matcher to find correspondence
        std::vector<std::vector<cv::DMatch>> matches;
        std::vector<cv::KeyPoint> objMatch, frameMatch;

        // 2.1. get all the matches between object and frame kps based on desc
        matcher->knnMatch(objIter->desc, frameDesc, matches, knnParam);

        // 2.2. add close-enough matches by distance (filtered matches)
        for (unsigned i = 0; i < matches.size(); i++)
        {
          if (matches[i][0].distance < knnMatchRatio * matches[i][1].distance) // 1st and 2nd diff distance check
          {
            objMatch.push_back(objIter->kps[matches[i][0].queryIdx]);
            frameMatch.push_back(frameKps[matches[i][0].trainIdx]);
          }
        }

        // 3. get the homography from the matches
        cv::Mat homography;
        cv::Mat inlierMask;
        std::vector<cv::KeyPoint> objInlier, frameInlier;
        std::vector<cv::DMatch> inlierMatches;

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
            objIter->isDetected = true;
            perspectiveTransform(objIter->rect2D, objIter->location2D, homography);
            mxre::cv_utils::drawBoundingBox(frame.cvMat, objIter->location2D);
          }
          else
            objIter->isDetected = false;
        }
      }

      out_frame = frame;
      out_obj_info = objInfos;

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

  } // namespace kernels
} // namespace mxre

