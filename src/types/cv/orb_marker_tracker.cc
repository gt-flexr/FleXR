#include <types/cv/orb_marker_tracker.h>

namespace mxre
{
  namespace cv_types
  {

    void ORBMarkerTracker::registerObject(const cv::Mat frame, cv::Rect roiRect) {
      // get the object mask
      std::vector<cv::Point2f> roiPoints;
      roiPoints.push_back(cv::Point2f(static_cast<float>(roiRect.x), static_cast<float>(roiRect.y)));
      roiPoints.push_back(cv::Point2f(static_cast<float>(roiRect.x + roiRect.width), static_cast<float>(roiRect.y)));
      roiPoints.push_back(cv::Point2f(static_cast<float>(roiRect.x + roiRect.width),
                                      static_cast<float>(roiRect.y + roiRect.height)));
      roiPoints.push_back(cv::Point2f(static_cast<float>(roiRect.x), static_cast<float>(roiRect.y + roiRect.height)));

      cv::Point *roiMask = new cv::Point[roiPoints.size()];
      const cv::Point *roiMaskHead = {&roiMask[0]};
      int roiPointNum = static_cast<int>(roiPoints.size());

      for (size_t i = 0; i < roiPoints.size(); i++)
      {
        roiMask[i].x = static_cast<int>(roiPoints[i].x);
        roiMask[i].y = static_cast<int>(roiPoints[i].y);
      }
      cv::Mat maskFrame = cv::Mat::zeros(frame.size(), CV_8UC1);
      cv::fillPoly(maskFrame, &roiMaskHead, &roiPointNum, 1, cv::Scalar::all(255));

      std::vector<cv::Point3f> rect3D;
      rect3D.push_back(cv::Point3f(0, 0, 0));
      rect3D.push_back(cv::Point3f(roiRect.width, 0, 0));
      rect3D.push_back(cv::Point3f(roiRect.width, roiRect.height, 0));
      rect3D.push_back(cv::Point3f(0, roiRect.height, 0));

      ObjectInfo newInfo;
      newInfo.img = frame(roiRect);
      newInfo.rect3D = rect3D;
      newInfo.rect2D = roiPoints;
      detector->detectAndCompute(frame, maskFrame, newInfo.kps, newInfo.desc);
      newInfo.index = numOfObjs++;

      objInfo.push_back(newInfo);

      delete[] roiMask;
    }


    void ORBMarkerTracker::printRegisteredObjects() {
      std::cout << "===== printRegisteredObjects =====" << std::endl;
      for(std::vector<ObjectInfo>::iterator it = objInfo.begin(); it != objInfo.end(); ++it) {
        printf("%dth Object Info \n", it->index);
        std::cout << "\tROI size: " << it->img.size() << std::endl;
        if (it->location2D.size() == 4)
        {
          printf("\tROI Location: 0(%f, %f), 1(%f, %f), 2(%f, %f), 3(%f, %f) \n",
                 it->location2D[0].x, it->location2D[0].y,
                 it->location2D[1].x, it->location2D[1].y,
                 it->location2D[2].x, it->location2D[2].y,
                 it->location2D[3].x, it->location2D[3].y);
        }
        printf("\t# of kps: %d\n", (int)it->kps.size());
      }
      std::cout << "==================================" << std::endl;
    }

  } // namespace cv_types
} // namespace mxre
