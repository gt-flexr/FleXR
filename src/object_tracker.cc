#include <include/object_tracker.h>

namespace mxre
{
  namespace cv_units
  {
    void ObjectTracker::registerObject(const cv::Mat frame, cv::Rect roiRect) {
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

      ObjectInfo newInfo;
      newInfo.img = frame(roiRect);
      newInfo.roi = roiPoints;
      detector->detectAndCompute(frame, maskFrame, newInfo.kps, newInfo.desc);
      newInfo.index = numOfObjs;

      numOfObjs += 1;
      objInfo.push_back(newInfo);

      delete[] roiMask;
    }

    void ObjectTracker::printRegisteredObjects() {
      std::cout << "===== printRegisteredObjects =====" << std::endl;
      for(std::vector<ObjectInfo>::iterator it = objInfo.begin(); it != objInfo.end(); ++it) {
        printf("%dth Object Info \n", it->index);
        std::cout << "\tROI size: " << it->img.size() << std::endl;
        printf("\tROI Location: 0(%f, %f), 1(%f, %f), 2(%f, %f), 3(%f, %f) \n", it->roi[0].x, it->roi[0].y,
               it->roi[1].x, it->roi[1].y,
               it->roi[2].x, it->roi[2].y,
               it->roi[3].x, it->roi[3].y);
        printf("\t# of kps: %d\n", (int)it->kps.size());
      }
      std::cout << "==================================" << std::endl;
    }
  } // namespace cv_units
} // namespace mxre