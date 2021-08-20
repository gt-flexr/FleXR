#include <components/orb_marker_tracker.h>

namespace flexr
{
  namespace components
  {
    OrbMarkerTracker::OrbMarkerTracker(): numOfObjs(0) {
      detector = cv::ORB::create();
      matcher  = cv::DescriptorMatcher::create("BruteForce-Hamming");
    }


    void OrbMarkerTracker::registerObject(const cv::Mat frame, cv::Rect roiRect) {
      cv::Point roiMask[4];

      roiMask[0] = cv::Point2f(static_cast<float>(roiRect.x), static_cast<float>(roiRect.y));
      roiMask[1] = cv::Point2f(static_cast<float>(roiRect.x + roiRect.width), static_cast<float>(roiRect.y));
      roiMask[2] = cv::Point2f(static_cast<float>(roiRect.x + roiRect.width), static_cast<float>(roiRect.y + roiRect.height));
      roiMask[3] = cv::Point2f(static_cast<float>(roiRect.x), static_cast<float>(roiRect.y + roiRect.height));

      const cv::Point *roiMaskHead = roiMask;
      int roiPointNum = 4;
      cv::Mat maskFrame = cv::Mat::zeros(frame.size(), CV_8UC1);
      cv::fillPoly(maskFrame, &roiMaskHead, &roiPointNum, 1, cv::Scalar::all(255));

      cv_types::MarkerInfo newMarker;
      newMarker.defaultLocationIn3D.push_back(cv::Point3f(0, 0, 0));
      newMarker.defaultLocationIn3D.push_back(cv::Point3f(roiRect.width, 0, 0));
      newMarker.defaultLocationIn3D.push_back(cv::Point3f(roiRect.width, roiRect.height, 0));
      newMarker.defaultLocationIn3D.push_back(cv::Point3f(0, roiRect.height, 0));

      newMarker.defaultLocationIn2D.push_back(cv::Point2f(0, 0));
      newMarker.defaultLocationIn2D.push_back(cv::Point2f(roiRect.width, 0));
      newMarker.defaultLocationIn2D.push_back(cv::Point2f(roiRect.width, roiRect.height));
      newMarker.defaultLocationIn2D.push_back(cv::Point2f(0, roiRect.height));

      newMarker.img = frame(roiRect);
      detector->detectAndCompute(frame, maskFrame, newMarker.kps, newMarker.desc);
      newMarker.index = numOfObjs++;

      markerInfo.push_back(newMarker);
    }


    void OrbMarkerTracker::printRegisteredObjects() {
      std::cout << "===== printRegisteredObjects =====" << std::endl;
      for(std::vector<cv_types::MarkerInfo>::iterator it = markerInfo.begin(); it != markerInfo.end(); ++it) {
        printf("%dth Object Info \n", it->index);
        std::cout << "\tROI size: " << it->img.size() << std::endl;
        if (it->defaultLocationIn2D.size() == 4)
        {
          printf("\tROI Location: 0(%f, %f), 1(%f, %f), 2(%f, %f), 3(%f, %f) \n",
                 it->defaultLocationIn2D[0].x, it->defaultLocationIn2D[0].y,
                 it->defaultLocationIn2D[1].x, it->defaultLocationIn2D[1].y,
                 it->defaultLocationIn2D[2].x, it->defaultLocationIn2D[2].y,
                 it->defaultLocationIn2D[3].x, it->defaultLocationIn2D[3].y);
        }
        printf("\tkps: %d, descs: %d\n", (int)it->kps.size(), (int)it->desc.total());
      }
      std::cout << "==================================" << std::endl;
    }


    void OrbMarkerTracker::setMarkerFromImages(std::string path, std::string stemName,
                                               int startIndex, int maxPlaceValue)
    {
      cv::Mat image;

      while(1) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(maxPlaceValue);
        ss << startIndex++;
        std::string imagePath = path + stemName + ss.str() + ".png";
        image = cv::imread(imagePath);
        if(image.empty()) {
          break;
        }

        cv::Rect roiRect(0, 0, image.cols, image.rows);
        registerObject(image, roiRect);
      }
      printRegisteredObjects();
    }


    void OrbMarkerTracker::setMarkerFromImages(std::string path, int startIndex, int maxPlaceValue)
    {
      cv::Mat image;

      while(1) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(maxPlaceValue);
        ss << startIndex++;
        std::string imagePath = path + ss.str() + ".png";
        image = cv::imread(imagePath);
        if(image.empty()) {
          break;
        }

        cv::Rect roiRect(0, 0, image.cols, image.rows);
        registerObject(image, roiRect);
      }
      printRegisteredObjects();
    }


    void OrbMarkerTracker::setMarkerFromImage(std::string path)
    {
      cv::Mat image;
      image = cv::imread(path);
      if(image.empty()) {
        debug_print("invalid path");
        return;
      }

      cv::Rect roiRect(0, 0, image.cols, image.rows);
      registerObject(image, roiRect);
      printRegisteredObjects();
    }


  } // namespace cv_types
} // namespace flexr

