#include <types/cv/orb_marker_tracker.h>

namespace mxre
{
  namespace cv_types
  {
    ORBMarkerTracker::ORBMarkerTracker(): numOfObjs(0) {
      detector = cv::ORB::create();
      matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
    }


    void ORBMarkerTracker::registerObject(const cv::Mat frame, cv::Rect roiRect) {
      cv::Point roiMask[4];

      roiMask[0] = cv::Point2f(static_cast<float>(roiRect.x), static_cast<float>(roiRect.y));
      roiMask[1] = cv::Point2f(static_cast<float>(roiRect.x + roiRect.width), static_cast<float>(roiRect.y));
      roiMask[2] = cv::Point2f(static_cast<float>(roiRect.x + roiRect.width), static_cast<float>(roiRect.y + roiRect.height));
      roiMask[3] = cv::Point2f(static_cast<float>(roiRect.x), static_cast<float>(roiRect.y + roiRect.height));

      const cv::Point *roiMaskHead = roiMask;
      int roiPointNum = 4;
      cv::Mat maskFrame = cv::Mat::zeros(frame.size(), CV_8UC1);
      cv::fillPoly(maskFrame, &roiMaskHead, &roiPointNum, 1, cv::Scalar::all(255));

      MarkerInfo newMarker;
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


    void ORBMarkerTracker::printRegisteredObjects() {
      std::cout << "===== printRegisteredObjects =====" << std::endl;
      for(std::vector<MarkerInfo>::iterator it = markerInfo.begin(); it != markerInfo.end(); ++it) {
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

  } // namespace cv_types
} // namespace mxre

