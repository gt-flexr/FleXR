#ifdef __FLEXR_KERNEL_ORB_CAM_LOCATOR__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {
    /* Constructor */
    OrbCamLocator::OrbCamLocator(std::string id, std::string markerPath, int camWidth, int camHeight): FleXRKernel(id)
    {
      setName("OrbCamLocator");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);
      portManager.registerOutPortTag("out_cam_pose", utils::sendLocalBasicCopy<OrbCamLocatorOutPose>);

      knnMatchRatio = 0.95f;
      knnParam = 3;
      ransacThresh = 5.0f;
      minInlierThresh = 20;
      detectionThresh = 0.6f;

      cv::Mat tempIntrinsic(3, 3, CV_64FC1);
      cv::Mat tempDistCoeffs(4, 1, CV_64FC1, {0, 0, 0, 0});
      tempIntrinsic.at<double>(0, 0) = width;
      tempIntrinsic.at<double>(0, 1) = 0;
      tempIntrinsic.at<double>(0, 2) = width/2;
      tempIntrinsic.at<double>(1, 0) = 0;
      tempIntrinsic.at<double>(1, 1) = width;
      tempIntrinsic.at<double>(1, 2) = height/2;
      tempIntrinsic.at<double>(2, 0) = 0;
      tempIntrinsic.at<double>(2, 1) = 0;
      tempIntrinsic.at<double>(2, 2) = 1;
      width = camWidth; height = camHeight;
      camIntrinsic = tempIntrinsic.clone();
      camDistCoeffs = tempDistCoeffs.clone();

      detector = cv::ORB::create(1000);
      matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
      // matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);

      // Register a marker to track
      cv::Mat markerImg = cv::imread(markerPath);
      if(markerImg.empty()) {
        debug_print("Invalid marker image path.");
        exit(0);
      }

      cv::Rect roiRect(0, 0, markerImg.cols-1, markerImg.rows-1);

      markerCorner3D.push_back(cv::Point3f(0,                          0, 0));
      markerCorner3D.push_back(cv::Point3f(roiRect.width,              0, 0));
      markerCorner3D.push_back(cv::Point3f(roiRect.width, roiRect.height, 0));
      markerCorner3D.push_back(cv::Point3f(0,             roiRect.height, 0));

      markerCorner2D.push_back(cv::Point2f(0,             0));
      markerCorner2D.push_back(cv::Point2f(roiRect.width, 0));
      markerCorner2D.push_back(cv::Point2f(roiRect.width, roiRect.height));
      markerCorner2D.push_back(cv::Point2f(0,             roiRect.height));

      cv::Mat markerImgGray = markerImg;
      cv::cvtColor(markerImgGray, markerImgGray, cv::COLOR_RGB2GRAY);
      detector->detectAndCompute(markerImgGray, cv::noArray(), markerKps, markerDesc);
    }


    raft::kstatus OrbCamLocator::run()
    {
      OrbCamLocatorInFrame *inFrame = portManager.getInput<OrbCamLocatorInFrame>("in_frame");
      OrbCamLocatorOutPose *outCamPose = portManager.getOutputPlaceholder<OrbCamLocatorOutPose>("out_cam_pose");

      outCamPose->setHeader(inFrame->tag, inFrame->seq, inFrame->ts, sizeof(OrbCamLocatorOutPose));

      double st = getTsNow();
      std::vector<cv::KeyPoint> frameKps;
      cv::Mat frameDesc;

      // 0. prepare gary frame
      cv::Mat grayFrame = inFrame->data.useAsCVMat();
      cv::cvtColor(grayFrame, grayFrame, cv::COLOR_RGB2GRAY);

      // 1. figure out frame keypoints and descriptors to detect objects in the frame
      detector->detectAndCompute(grayFrame, cv::noArray(), frameKps, frameDesc);

      // 2. use the matcher to find correspondence
      std::vector<std::vector<cv::DMatch>> knnMatches;
      std::vector<cv::KeyPoint> matchingMarkerKps, markerKpsInFrame;
      cv::Mat homography;
      cv::Mat inlierMask;
      std::vector<cv::KeyPoint> objInlier, frameInlier;
      std::vector<cv::DMatch> closeMatches;

      // 2.1. get all the matches between object and frame kps based on desc
      matcher->knnMatch(markerDesc, frameDesc, knnMatches, knnParam);

      // 2.2. add close-enough matches by distance (filtered matches)
      for (unsigned i = 0; i < knnMatches.size(); i++)
      {
        if (knnMatches[i][0].distance < knnMatchRatio * knnMatches[i][1].distance) // 1st and 2nd diff distance check
        {
          matchingMarkerKps.push_back(markerKps[knnMatches[i][0].queryIdx]);
          markerKpsInFrame.push_back(frameKps[knnMatches[i][0].trainIdx]);
        }
      }

      debug_print("markerKps (%ld), frameKps (%ld), matchingKps (%ld)",
                  markerKps.size(), frameKps.size(), matchingMarkerKps.size());

      // 3. get the homography from the matches
      if (markerKpsInFrame.size() >= 4)
      {
        homography = findHomography(cv_utils::convertKpsToPts(matchingMarkerKps),
                                    cv_utils::convertKpsToPts(markerKpsInFrame),
                                    cv::RANSAC, ransacThresh, inlierMask);

        // 3.1. check matching keypoints with detectionThresh percentage
        if(!homography.empty() && matchingMarkerKps.size() > markerKps.size()*detectionThresh)
        {
          // 3.2. get the translation and rotation
          std::vector<cv::Point2f> markerCornersInFrame(4);
          perspectiveTransform(markerCorner2D, markerCornersInFrame, homography);

          cv::Mat rvec, tvec;
          cv::solvePnPRansac(markerCorner3D, markerCornersInFrame, camIntrinsic, camDistCoeffs, rvec, tvec);
          float tx, ty, tz;
          float rx, ry, rz;

          tx = tvec.at<float>(0, 0); ty = tvec.at<float>(0, 1); ty = tvec.at<float>(0, 2);
          rx = rvec.at<float>(0, 0); ry = rvec.at<float>(0, 1); ry = rvec.at<float>(0, 2);

          tx = (tx - width/2)  / (width);
          ty = (ty - width/2)  / (width);
          tz = -0.5;

          debug_print("tx(%f), ty(%f), tz(%f)", tx, ty, tz);
          debug_print("rx(%f), ry(%f), rz(%f)", rx, ry, rz);

          outCamPose->data.tx = tx;
          outCamPose->data.ty = ty;
          outCamPose->data.tz = tz;
          outCamPose->data.rx = rx;
          outCamPose->data.ry = ry;
          outCamPose->data.rz = rz;
          double et = getTsNow();
          if(logger.isSet()) logger.getInstance()->info("{}\t {}\t {}", st, et, et-st);
          portManager.sendOutput("out_cam_pose", outCamPose);
        }
        else
        {
          debug_print("detection percentage (%f) is less than threshold (%f)",
                     (float)matchingMarkerKps.size()/(float)markerKps.size(), detectionThresh);
        }
      }

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);

      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

#endif

