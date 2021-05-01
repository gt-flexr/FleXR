#include <mxre>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#define LATENCY_BREAKDOWN 1

using namespace std;

int main(int argc, char const *argv[])
{
  string mxre_home = getenv("MXRE_HOME");
  string config_yaml = mxre_home + "/examples/marker_ar/config.yaml";

  if(mxre_home.empty()) {
    cout << "Set MXRE_HOME as a environment variable" << endl;
    return 0;
  }
  else cout << config_yaml << endl;

  YAML::Node config = YAML::LoadFile(config_yaml);
  string markerPath    = config["marker_path"].as<string>();
  int width            = config["width"].as<int>();
  int height           = config["height"].as<int>();
  string bagFile       = config["bag_file"].as<string>();
  string bagTopic      = config["bag_topic"].as<string>();
  int bagFPS           = config["bag_fps"].as<int>();
  if(markerPath.empty() || bagFile.empty() || bagTopic.empty() || bagFPS == 0) {
    debug_print("Please put correct info on config.yaml");
    return -1;
  }


  /*
   *  Load a marker from an image
   */
  mxre::cv_types::ORBMarkerTracker orbMarkerTracker;
  mxre::cv_utils::setMarkerFromImages(markerPath + "/", 0, 1, orbMarkerTracker);
  std::vector<mxre::cv_types::MarkerInfo> registeredMarkers = orbMarkerTracker.getRegisteredObjects();


  /*
   *  Cache frames from a bag file
   */
  mxre::components::ROSBagFrameReader bagFrameReader(bagFile, bagTopic);
  bagFrameReader.cacheFrames(400, 400);


  /*
   *  ORB detector & matcher, set matching params
   */
  int frameIndex = 0;
  double frameTimestamp;
  cv::Ptr<cv::Feature2D> detector = cv::ORB::create();
  cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
  double knnMatchRatio = 0.8f;
  int knnParam = 5;
  double ransacThresh = 2.5f;
  int minInlierThresh = 20;

  /*
   *  Set default camera params
   */
  cv::Mat camIntrinsic(3, 3, CV_64FC1);
  camIntrinsic.at<double>(0, 0) = width; camIntrinsic.at<double>(0, 1) = 0; camIntrinsic.at<double>(0, 2) = width/2;
  camIntrinsic.at<double>(1, 0) = 0; camIntrinsic.at<double>(1, 1) = width; camIntrinsic.at<double>(1, 2) = height/2;
  camIntrinsic.at<double>(2, 0) = 0; camIntrinsic.at<double>(2, 1) = 0; camIntrinsic.at<double>(2, 2) = 1;
  cv::Mat camDistCoeffs(4, 1, CV_64FC1, {0, 0, 0, 0});

  /*
   *  Set rendering contexts
   */
  mxre::egl_types::pbuffer *pbuf;
  mxre::ar_types::VirtualWorldManager worldManager;
  GLuint backgroundTexture;

  pbuf = new mxre::egl_types::pbuffer;
  mxre::egl_utils::initPbuffer(*pbuf, width, height);

  mxre::egl_utils::bindPbuffer(*pbuf);
  mxre::gl_utils::initGL(width, height);
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
  auto logger = spdlog::basic_logger_st("marker_ar_orb_serialized", "logs/marker_ar_orb_serialized.log");
  double e2eStart, e2eEnd;
  double blockStart, blockEnd;
#endif


  /*
   *  Processing Loop
   */
  while(1) {
    /*
     *  1. Load camera frames
     */
#ifdef __PROFILE__
    e2eStart = getTimeStampNow();
    blockStart = e2eStart;
#endif

    // Camera Frequency
    std::this_thread::sleep_for(std::chrono::milliseconds(1000/bagFPS-1));
    mxre::types::Frame frame = bagFrameReader.getNextFrame();
    frameTimestamp = getTimeStampNow();

#ifdef LATENCY_BREAKDOWN
    blockEnd = getTimeStampNow();
    logger->info("\tCamera Read Time\t{}", blockEnd - blockStart);
    blockStart = blockEnd;
#endif

    /*
     *  2. Detect registered markers on the loaded frame
     */
    std::vector<cv::KeyPoint> frameKps;
    cv::Mat frameDesc;
    std::vector<mxre::cv_types::DetectedMarker> detectedMarkers;
    std::vector<mxre::gl_types::ObjectContext> markerContexts;

    // 2.1. Get frame keypoints and descriptors
    cv::Mat grayFrame = frame.useAsCVMat().clone();
    cv::cvtColor(grayFrame, grayFrame, CV_BGR2GRAY);
    detector->detectAndCompute(grayFrame, cv::noArray(), frameKps, frameDesc);

#ifdef LATENCY_BREAKDOWN
    blockEnd = getTimeStampNow();
    logger->info("\tKeypoint Extraction Time\t{}", blockEnd - blockStart);
    blockStart = blockEnd;
#endif

    // multiple object detection
    std::vector<mxre::cv_types::MarkerInfo>::iterator markerInfo;
    for (markerInfo = registeredMarkers.begin(); markerInfo != registeredMarkers.end(); ++markerInfo)
    {
      std::vector<std::vector<cv::DMatch>> matches;
      std::vector<cv::KeyPoint> objMatch, frameMatch;
      cv::Mat homography;
      cv::Mat inlierMask;
      std::vector<cv::KeyPoint> objInlier, frameInlier;
      std::vector<cv::DMatch> inlierMatches;

      // 2.2. find matching descriptors between the frame and the object
      matcher->knnMatch(markerInfo->desc, frameDesc, matches, knnParam);

#ifdef LATENCY_BREAKDOWN
      blockEnd = getTimeStampNow();
      logger->info("\tFinding Matching Descriptors Time\t{}", blockEnd - blockStart);
      blockStart = blockEnd;
#endif

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

#ifdef LATENCY_BREAKDOWN
    blockEnd = getTimeStampNow();
    logger->info("\tFiltering and Detection Time\t{}", blockEnd - blockStart);
    blockStart = blockEnd;
#endif

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
      float transX = (tvec.at<double>(0, 0) * 2) / width;
      float transY = (tvec.at<double>(0, 1) * 2) / height;
      float transZ = tvec.at<double>(0, 2) / width;

      float rotX = rvec.at<double>(0, 0);
      float rotY = rvec.at<double>(0, 1);
      float rotZ = rvec.at<double>(0, 2);

      markerContext.rvec.x = rotX;   markerContext.rvec.y = -rotY;   markerContext.rvec.z = -rotZ;
      markerContext.tvec.x = transX; markerContext.tvec.y = transY; markerContext.tvec.z = -transZ;
      markerContexts.push_back(markerContext);
    }

#ifdef LATENCY_BREAKDOWN
    blockEnd = getTimeStampNow();
    logger->info("\tReal-Virtual Mapping Time\t{}", blockEnd - blockStart);
    blockStart = getTimeStampNow();
#endif

    /********************************
                Overlay Objects
    *********************************/
    //mxre::types::Frame mxreFrame(cachedFrame, frameIndex, frameTimestamp);
    if(glIsTexture(backgroundTexture))
      mxre::gl_utils::updateTextureFromFrame(&frame, backgroundTexture);
    else
      mxre::gl_utils::makeTextureFromFrame(&frame, backgroundTexture);
    frame.release();

    // 2. Draw background frame
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mxre::gl_utils::startBackground(width, height);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    glBegin(GL_QUADS);
    glColor3f(1, 1, 1);
    glTexCoord2i(0,0); glVertex3f(0,     0,      -1);
    glTexCoord2i(1,0); glVertex3f(width, 0,      -1);
    glTexCoord2i(1,1); glVertex3f(width, height, -1);
    glTexCoord2i(0,1); glVertex3f(0,     height, -1);
    glEnd();
    mxre::gl_utils::endBackground();

    worldManager.startWorlds('f', markerContexts);

    mxre::types::Frame resultFrame = mxre::gl_utils::exportGLBufferToCV(width, height, frameIndex, frameTimestamp);
    frameIndex++;

#ifdef LATENCY_BREAKDOWN
    blockEnd = getTimeStampNow();
    logger->info("\tRendering and Overlaying Time\t{}", blockEnd - blockStart);
    blockStart = getTimeStampNow();
#endif

    //cv::imshow("CVDisplay", resultFrame.useAsCVMat());
    //int inKey = cv::waitKey(1) & 0xFF;

#ifdef LATENCY_BREAKDOWN
    blockEnd = getTimeStampNow();
    logger->info("\tDisplaying Time\t{}", blockEnd - blockStart);
#endif

#ifdef __PROFILE__
    e2eEnd = getTimeStampNow();
    logger->info("E2E Time\t{}\tFPS: {}", e2eEnd - e2eStart, 1000/(e2eEnd - e2eStart));
    debug_print("clock / %dth frameTimstamp: %f / %f", resultFrame.index, e2eEnd - e2eStart,
                e2eEnd - resultFrame.timestamp);
#endif

    resultFrame.release();
  }

  return 0;
}

