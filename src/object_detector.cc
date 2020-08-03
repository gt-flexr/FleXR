#include <include/object_detector.h>

namespace mxre
{
  namespace pipeline
  {
    namespace ctx_understanding
    {
      ObjectDetector::ObjectDetector(std::vector<mxre::cv_units::ObjectInfo> registeredObjVec,
                                     cv::Ptr<cv::Feature2D> _detector,
                                     cv::Ptr<cv::DescriptorMatcher> _matcher) : objInfoVec(registeredObjVec),
                                                                                detector(_detector),
                                                                                matcher(_matcher), raft::kernel()
      {
        input.addPort<cv::Mat>("in_frame");
        input.addPort<clock_t>("in_timestamp");

        output.addPort<cv::Mat>("out_frame");
        output.addPort<std::vector<mxre::cv_units::ObjectInfo>>("out_obj_info");
        output.addPort<clock_t>("out_timestamp");

        // Object Detection Parameters
        knnMatchRatio = 0.8f;
        knnParam = 5;
        ransacThresh = 2.5f;
        minInlierThresh = 10;
      }


      ObjectDetector::~ObjectDetector() {}


      raft::kstatus ObjectDetector::run() {
        std::vector<cv::KeyPoint> frameKps;
        cv::Mat frameDesc;
        // get cam frame & timestamp as inputs from the previous kernel
        auto frame = input["in_frame"].peek<cv::Mat>();
        auto in_st = input["in_timestamp"].peek<clock_t>();
        clock_t rt = clock();
        printf("[ObjectDetector] Cam->ObjDetector communication cost %f ms \n", ((float)(rt) - in_st)/CLOCKS_PER_SEC*1000);

        // prepare output for the next kernel
        auto out_frame = output["out_frame"].template allocate_s<cv::Mat>();
        auto out_obj_info = output["out_obj_info"].template allocate_s<std::vector<mxre::cv_units::ObjectInfo>>();
        auto out_ts = output["out_timestamp"].template allocate_s<clock_t>();

        // 1. figure out frame keypoints and descriptors to detect objects in the frame
        detector->detectAndCompute(frame, cv::noArray(), frameKps, frameDesc);
        clock_t temp1 = clock();
        printf("[ObjectDetector] frame detectAndCompute %f ms \n", ((float)(temp1) - rt)/CLOCKS_PER_SEC*1000);

        // multiple object detection
        std::vector<mxre::cv_units::ObjectInfo>::iterator objIter;
        for (objIter = objInfoVec.begin(); objIter != objInfoVec.end(); ++objIter)
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

          clock_t temp2 = clock();
          printf("[ObjectDetector] matcher %f ms \n", ((float)(temp2) - temp1)/CLOCKS_PER_SEC*1000);

          // 3. get the homography from the matches
          cv::Mat homography;
          cv::Mat inlierMask;
          std::vector<cv::KeyPoint> objInlier, frameInlier;
          std::vector<cv::DMatch> inlierMatches;

          if (objMatch.size() >= 4)
          {
            homography = findHomography(mxre::cv_units::convertKpsToPts(objMatch),
                                                 mxre::cv_units::convertKpsToPts(frameMatch),
                                                 cv::RANSAC, ransacThresh, inlierMask);
          }

          clock_t temp3 = clock();
          printf("[ObjectDetector] findHomography %f ms \n", ((float)(temp3) - temp2)/CLOCKS_PER_SEC*1000);

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
            clock_t temp4 = clock();
            printf("[ObjectDetector] find inliers %f ms \n", ((float)(temp4)-temp3) / CLOCKS_PER_SEC * 1000);

            // 5. Draw the object rectangle in the frame via homography and inliers
            if (objInlier.size() >= minInlierThresh)
            {
              perspectiveTransform(objIter->roi, objIter->location, homography);
              mxre::cv_units::drawBoundingBox(frame, objIter->location);
            }
            clock_t temp5 = clock();
            printf("[ObjectDetector] perspectiveTransform & draw Box %f ms \n", ((float)(temp5)-temp4) / CLOCKS_PER_SEC * 1000);
          }
        }

        *out_frame = frame;
        *out_obj_info = objInfoVec;
        *out_ts = clock();

        input["in_frame"].recycle();
        input["in_timestamp"].recycle();

        printf("[ObjectDetector] exe time %f ms \n", ((float)(*out_ts) - in_st)/CLOCKS_PER_SEC*1000);
        output["out_frame"].send();
        output["out_obj_info"].send();
        output["out_timestamp"].send();
        return raft::proceed;
      }

    } // namespace ctx_understanding
  }   // namespace pipeline
} // namespace mxre