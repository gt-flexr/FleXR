#include <opencv2/core/mat.hpp>
#include <raft>
#include <mxre>
#include <bits/stdc++.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#define WIDTH 1920
#define HEIGHT 1080

using namespace std;


int main(int argc, char const *argv[])
{
  /*
   *  Load a marker from an image
   */
  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  //mxre::cv_utils::setMarkerFromImages("/home/jin/github/mxre/resources/markers/", "720p_marker", 0, 1, orbMarkerTracker);
  mxre::cv_utils::setMarkerFromImages("/home/jin/github/mxre/resources/markers/", "1080p_marker", 0, 1, orbMarkerTracker);
  //mxre::cv_utils::setMarkerFromImages("/home/jin/github/mxre/resources/markers/", "480p_marker", 0, 1, orbMarkerTracker);
  std::vector<mxre::cv_types::MarkerInfo> registeredMarkers = orbMarkerTracker.getRegisteredObjects();
  orbMarkerTracker.printRegisteredObjects();

  /*
   *  ORB detector & matcher, set matching params
   */
  int frame_idx = 1;
  cv::Ptr<cv::cuda::ORB> detector = cv::cuda::ORB::create();
  cv::Ptr<cv::cuda::DescriptorMatcher> matcher = cv::cuda::DescriptorMatcher::createBFMatcher(cv::NORM_HAMMING);
  double knnMatchRatio = 0.8f;
  int knnParam = 5;
  double ransacThresh = 2.5f;
  int minInlierThresh = 20;

  /*
   *  Set default camera params
   */
  cv::Mat camIntrinsic(3, 3, CV_64FC1);
  camIntrinsic.at<double>(0, 0) = WIDTH; camIntrinsic.at<double>(0, 1) = 0; camIntrinsic.at<double>(0, 2) = WIDTH/2;
  camIntrinsic.at<double>(1, 0) = 0; camIntrinsic.at<double>(1, 1) = WIDTH; camIntrinsic.at<double>(1, 2) = HEIGHT/2;
  camIntrinsic.at<double>(2, 0) = 0; camIntrinsic.at<double>(2, 1) = 0; camIntrinsic.at<double>(2, 2) = 1;
  cv::Mat camDistCoeffs(4, 1, CV_64FC1, {0, 0, 0, 0});

  /*
   *  Set rendering contexts
   */
  mxre::egl_types::pbuffer *pbuf;
  mxre::ar_types::VirtualWorldManager worldManager;
  GLuint backgroundTexture;

  pbuf = new mxre::egl_types::pbuffer;
  mxre::egl_utils::initPbuffer(*pbuf, WIDTH, HEIGHT);

  mxre::egl_utils::bindPbuffer(*pbuf);
  mxre::gl_utils::initGL(WIDTH, HEIGHT);
  worldManager.initShader();

  /*
   *  Set a virtual world to each marker
   */
  std::vector<mxre::cv_types::MarkerInfo>::iterator markerInfo;
  for (markerInfo = registeredMarkers.begin(); markerInfo != registeredMarkers.end(); ++markerInfo) {
    worldManager.addWorld(markerInfo->index);
  }
  printf("NumOfWorlds %d\n", worldManager.numOfWorlds);
  for(int i = 0; i < worldManager.numOfWorlds; i++) {
    worldManager.addObjectToWorld(i);
  }
  //mxre::egl_utils::unbindPbuffer(*pbuf);
#ifdef __PROFILE__
  auto logger = spdlog::basic_logger_st("serialized_example", "logs/serialized_example_with_cuda.log");
#endif

  cv::cuda::GpuMat cuFrame;
  cv::cuda::Stream stream;
  cv::cuda::GpuMat cuKp, cuDesc;
  cv::cuda::GpuMat cuMatches;


  while(1) {
    /*
     *  1. Load camera frames
     */
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(6);
    ss << frame_idx++;
    //std::string imagePath = "/home/jin/github/mxre/resources/video/480p/video_" + ss.str() + ".png";
    //std::string imagePath = "/home/jin/github/mxre/resources/video/720p/video_" + ss.str() + ".png";
    std::string imagePath = "/home/jin/github/mxre/resources/video/1080p/video_" + ss.str() + ".png";
    cv::Mat image = cv::imread(imagePath);
    if(image.empty()) {
      debug_print("Could not read the image: %s", imagePath.c_str());
      break;
    }

#ifdef __PROFILE__
    double startTimeStamp = getTimeStampNow();
#endif

    /*
     *  2. Detect registered markers on the loaded frame
     */
    std::vector<cv::KeyPoint> frameKps;
    std::vector<mxre::cv_types::DetectedMarker> detectedMarkers;
    std::vector<mxre::gl_types::ObjectContext> markerContexts;

    // 2.1. Get frame keypoints and descriptors
    cv::Mat grayFrame = image.clone();
    cv::cvtColor(grayFrame, grayFrame, CV_BGR2GRAY);
    cuFrame.upload(grayFrame);
    detector->detectAndComputeAsync(cuFrame, cv::noArray(), cuKp, cuDesc, false, stream);
    stream.waitForCompletion();
    detector->convert(cuKp, frameKps);

    // multiple object detection
    std::vector<mxre::cv_types::MarkerInfo>::iterator markerInfo;
    for (markerInfo = registeredMarkers.begin(); markerInfo != registeredMarkers.end(); ++markerInfo)
    {
      cv::cuda::GpuMat cuObjDesc;
      std::vector<std::vector<cv::DMatch>> matches;
      std::vector<cv::KeyPoint> objMatch, frameMatch;
      cv::Mat homography;
      cv::Mat inlierMask;
      std::vector<cv::KeyPoint> objInlier, frameInlier;
      std::vector<cv::DMatch> inlierMatches;


      // 2.2. find matching descriptors between the frame and the object
      cuObjDesc.upload(markerInfo->desc);
      matcher->knnMatchAsync(cuObjDesc, cuDesc, cuMatches, 2, cv::noArray(), stream);
      stream.waitForCompletion();
      matcher->knnMatchConvert(cuMatches, matches);

      // 2.3. filter matching descriptors by their distances, and store the pair of matching keypoints
      for (unsigned i = 0; i < matches.size(); i++)
      {
        if (matches[i][0].distance < knnMatchRatio * matches[i][1].distance)
        {
          objMatch.push_back(markerInfo->kps[matches[i][0].queryIdx]);
          frameMatch.push_back(frameKps[matches[i][0].trainIdx]);
        }
      }

      // 2.4. find the homography with the paired keypoints
      if (objMatch.size() >= 4)
      {
        homography = findHomography(mxre::cv_utils::convertKpsToPts(objMatch),
                                    mxre::cv_utils::convertKpsToPts(frameMatch),
                                    cv::RANSAC, ransacThresh, inlierMask);
      }

      // 2.5. filter the matching keypoints by checking homography inliers
      if (objMatch.size() >= 4 && !homography.empty())
      {
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

        // 2.6. if the number of inliers is bigger than threshold, detect!
        if (objInlier.size() >= minInlierThresh)
        {
          mxre::cv_types::DetectedMarker detectedMarker;
          detectedMarker.index = markerInfo->index;
          detectedMarker.defaultLocationIn3D = markerInfo->defaultLocationIn3D;
          perspectiveTransform(markerInfo->defaultLocationIn2D, detectedMarker.locationIn2D, homography);
          detectedMarkers.push_back(detectedMarker);
        }
      }
    }

    /********************************
                Extract Contexts
    *********************************/
    std::vector<mxre::cv_types::DetectedMarker>::iterator detectedMarker;
    for (detectedMarker = detectedMarkers.begin(); detectedMarker != detectedMarkers.end(); ++detectedMarker)
    {
      cv::Mat rvec, tvec;

      cv::solvePnPRansac(detectedMarker->defaultLocationIn3D, detectedMarker->locationIn2D,
          camIntrinsic, camDistCoeffs, rvec, tvec);

      mxre::gl_types::ObjectContext markerContext;
      markerContext.index = detectedMarker->index;
      float transX = (tvec.at<double>(0, 0) * 2) / WIDTH;
      float transY = (tvec.at<double>(0, 1) * 2) / HEIGHT;
      float transZ = tvec.at<double>(0, 2) / WIDTH;

      float rotX = rvec.at<double>(0, 0);
      float rotY = rvec.at<double>(0, 1);
      float rotZ = rvec.at<double>(0, 2);

      markerContext.rvec.x = rotX;   markerContext.rvec.y = -rotY;   markerContext.rvec.z = -rotZ;
      markerContext.tvec.x = transX; markerContext.tvec.y = transY; markerContext.tvec.z = -transZ;
      markerContexts.push_back(markerContext);
    }

    /********************************
                Overlay Objects
    *********************************/
    mxre::types::Frame mxreFrame(image);
    if(glIsTexture(backgroundTexture))
      mxre::gl_utils::updateTextureFromFrame(&mxreFrame, backgroundTexture);
    else
      mxre::gl_utils::makeTextureFromFrame(&mxreFrame, backgroundTexture);
    mxreFrame.release();

    // 2. Draw background frame
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mxre::gl_utils::startBackground(WIDTH, HEIGHT);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glBegin(GL_QUADS);
    glColor3f(1, 1, 1);
    glTexCoord2i(0,0); glVertex3f(0,     0,      -1);
    glTexCoord2i(1,0); glVertex3f(WIDTH, 0,      -1);
    glTexCoord2i(1,1); glVertex3f(WIDTH, HEIGHT, -1);
    glTexCoord2i(0,1); glVertex3f(0,     HEIGHT, -1);
    glEnd();
    mxre::gl_utils::endBackground();

    worldManager.startWorlds('f', markerContexts);

    mxre::types::Frame resultFrame = mxre::gl_utils::exportGLBufferToCV(WIDTH, HEIGHT);

#ifdef __PROFILE__
    double endTimeStamp = getTimeStampNow();
    logger->info("{}\t {}\t {}", startTimeStamp, endTimeStamp, endTimeStamp-startTimeStamp);
#endif

    cv::imshow("CVDisplay", resultFrame.useAsCVMat());
    int inKey = cv::waitKey(1) & 0xFF;
    resultFrame.release();
  }

  return 0;
}

