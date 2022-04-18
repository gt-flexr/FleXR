#ifdef __FLEXR_KERNEL_CUDA_ORB_CAM_LOCATOR__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace kernels
  {
    CudaOrbCamLocator::CudaOrbCamLocator(std::string id, std::string markerPath, int camWidth, int camHeight): FleXRKernel(id)
    {
      setName("CudaOrbCamLocator");
      portManager.registerInPortTag("in_frame", components::PortDependency::BLOCKING, utils::deserializeRawFrame);
      portManager.registerOutPortTag("out_cam_pose",
                                     utils::sendLocalBasicCopy<CudaOrbCamLocatorOutPose>);

      // Object Detection Parameters
      knnMatchRatio = 0.95f;
      knnParam = 3;
      ransacThresh = 5.0f;
      minInlierThresh = 20;
      detectionThresh = 0.6f;

      width  = camWidth;
      height = camHeight;

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
      camIntrinsic = tempIntrinsic.clone();
      camDistCoeffs = tempDistCoeffs.clone();

      detector = cv::cuda::ORB::create(1000);
      matcher = cv::cuda::DescriptorMatcher::createBFMatcher(cv::NORM_HAMMING);
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
      cv::Ptr<cv::Feature2D> tempDetector = cv::ORB::create(1000);
      cv::cvtColor(markerImgGray, markerImgGray, cv::COLOR_RGB2GRAY);
      tempDetector->detectAndCompute(markerImgGray, cv::noArray(), markerKps, markerDesc);
    }


    raft::kstatus CudaOrbCamLocator::run() {
      CudaOrbCamLocatorInFrame *inFrame = portManager.getInput<CudaOrbCamLocatorInFrame>("in_frame");
      CudaOrbCamLocatorOutPose *outCamPose = \
                                portManager.getOutputPlaceholder<CudaOrbCamLocatorOutPose>("out_cam_pose");

      outCamPose->setHeader(inFrame->tag, inFrame->seq, inFrame->ts, sizeof(CudaOrbCamLocatorOutPose));

      double st = getTsNow();
      std::vector<cv::KeyPoint> frameKps;

      // 1. prepare gary frame
      cv::Mat grayFrame = inFrame->data.useAsCVMat();
      cv::cvtColor(grayFrame, grayFrame, cv::COLOR_RGB2GRAY);
      cuFrame.upload(grayFrame);

      // 2. run CUDA ORB & convert the GPU result into CPU; cpu kps & gpu desc are ready
      detector->detectAndComputeAsync(cuFrame, cv::noArray(), cuKp, cuDesc, false, stream);
      stream.waitForCompletion();
      detector->convert(cuKp, frameKps);

      // 3. multi-obj detection
      cv::cuda::GpuMat cuObjDesc;
      std::vector<std::vector<cv::DMatch>> matches;
      std::vector<cv::KeyPoint> objMatch, frameMatch;
      cv::Mat homography;
      cv::Mat inlierMask;
      std::vector<cv::KeyPoint> objInlier, frameInlier;
      std::vector<cv::DMatch> inlierMatches;

      // 3-1. upload each objDesc into cuDesc
      cuObjDesc.upload(markerDesc);

      // 3-2. find matched descs for finding matching kps
      matcher->knnMatchAsync(cuObjDesc, cuDesc, cuMatches, knnParam, cv::noArray(), stream);
      stream.waitForCompletion();
      matcher->knnMatchConvert(cuMatches, matches);

      // 3-3. with matched descs, find corresponding keypoints
      for(unsigned i = 0; i < matches.size(); i++)
      {
        if(matches[i][0].distance < knnMatchRatio * matches[i][1].distance)
        {
          objMatch.push_back(markerKps[matches[i][0].queryIdx]);
          frameMatch.push_back(frameKps[matches[i][0].trainIdx]);
        }
      }

      debug_print("markerKps (%ld), frameKps (%ld), matchingKps (%ld)",
                  markerKps.size(), frameKps.size(), objMatch.size());

      // 4. Find the homography with matched kps (at least 4kps for planar obj)
      if(objMatch.size() >= 4)
      {
        homography = findHomography(flexr::cv_utils::convertKpsToPts(objMatch),
                                    flexr::cv_utils::convertKpsToPts(frameMatch),
                                    cv::RANSAC, ransacThresh, inlierMask);

        // 5. handle the detected object
        if(!homography.empty() && objMatch.size() > markerKps.size()*detectionThresh)
        {
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

          portManager.sendOutput("out_cam_pose", outCamPose);
        }
        else
        {
          debug_print("detection percentage (%f) is less than threshold (%f)",
                     (float)objMatch.size()/(float)markerKps.size(), detectionThresh);
        }
      }

      double et = getTsNow();
      if(logger.isSet()) logger.getInstance()->info("{}\t {}\t {}", st, et, et-st);

      inFrame->data.release();
      portManager.freeInput("in_frame", inFrame);

      freqManager.adjust();
      return raft::proceed;
    }

  } // namespace ctx_understanding
} // namespace flexr

#endif

