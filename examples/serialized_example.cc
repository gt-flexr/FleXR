#include <raft>
#include <mxre>
#include <bits/stdc++.h>

#define WIDTH 1280
#define HEIGHT 720

using namespace std;


int main(int argc, char const *argv[])
{
  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages("/home/jin/github/mxre/resources/markers/", "720p_marker", 0, 1, orbMarkerTracker);
  std::vector<mxre::cv_types::MarkerInfo> registeredMarkers = orbMarkerTracker.getRegisteredObjects();

  int frame_idx = 1;
  cv::Ptr<cv::Feature2D> detector = cv::ORB::create();
  cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");

  double knnMatchRatio = 0.8f;
  int knnParam = 5;
  double ransacThresh = 2.5f;
  int minInlierThresh = 20;

  cv::Mat camIntrinsic(3, 3, CV_64FC1);
  camIntrinsic.at<double>(0, 0) = WIDTH; camIntrinsic.at<double>(0, 1) = 0; camIntrinsic.at<double>(0, 2) = WIDTH/2;
  camIntrinsic.at<double>(1, 0) = 0; camIntrinsic.at<double>(1, 1) = WIDTH; camIntrinsic.at<double>(1, 2) = HEIGHT/2;
  camIntrinsic.at<double>(2, 0) = 0; camIntrinsic.at<double>(2, 1) = 0; camIntrinsic.at<double>(2, 2) = 1;
  cv::Mat camDistCoeffs(4, 1, CV_64FC1, {0, 0, 0, 0});

  mxre::egl_types::pbuffer *pbuf;
  mxre::ar_types::VirtualWorldManager worldManager;
  GLuint backgroundTexture;

  pbuf = new mxre::egl_types::pbuffer;
  mxre::egl_utils::initPbuffer(*pbuf, WIDTH, HEIGHT);

  mxre::egl_utils::bindPbuffer(*pbuf);
  mxre::gl_utils::initGL(WIDTH, HEIGHT);
  worldManager.initShader();
  std::vector<mxre::cv_types::MarkerInfo>::iterator markerInfo;
  for (markerInfo = registeredMarkers.begin(); markerInfo != registeredMarkers.end(); ++markerInfo) {
    worldManager.addWorld(markerInfo->index);
  }
  printf("NumOfWorlds %d\n", worldManager.numOfWorlds);
  for(int i = 0; i < worldManager.numOfWorlds; i++) {
    worldManager.addObjectToWorld(i);
  }
  //mxre::egl_utils::unbindPbuffer(*pbuf);


  while(1) {
    /********************************
               Load images
    *********************************/
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(6);
    ss << frame_idx++;
    std::string imagePath = "/home/jin/github/mxre/resources/video/720p/video_" + ss.str() + ".png";
    cv::Mat image = cv::imread(imagePath);
    if(image.empty()) {
      debug_print("Could not read the image: %s", imagePath.c_str());
      break;
    }

    /********************************
               Detect Markers
    *********************************/
    std::vector<cv::KeyPoint> frameKps;
    cv::Mat frameDesc;
    cv::Mat grayFrame = image.clone();
    std::vector<mxre::cv_types::DetectedMarker> detectedMarkers;
    std::vector<mxre::gl_types::ObjectContext> markerContexts;

    cv::cvtColor(grayFrame, grayFrame, CV_BGR2GRAY);
    detector->detectAndCompute(grayFrame, cv::noArray(), frameKps, frameDesc);

    // multiple object detection
    std::vector<mxre::cv_types::MarkerInfo>::iterator markerInfo;
    for (markerInfo = registeredMarkers.begin(); markerInfo != registeredMarkers.end(); ++markerInfo)
    {
      // 2. use the matcher to find correspondence
      std::vector<std::vector<cv::DMatch>> matches;
      std::vector<cv::KeyPoint> objMatch, frameMatch;

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

    cv::imshow("CVDisplay", resultFrame.useAsCVMat());
    int inKey = cv::waitKey(100) & 0xFF;
    resultFrame.release();
  }

  return 0;
}

