#ifndef __FLEXR_COMPONENT_ORB_MARKER_TRACKER__
#define __FLEXR_COMPONENT_ORB_MARKER_TRACKER__

#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>
#include "defs.h"
#include "types/cv/types.h"

namespace flexr
{
  namespace components
  {

    class OrbMarkerTracker
    {
    private:
      cv::Ptr<cv::Feature2D> detector;
      cv::Ptr<cv::DescriptorMatcher> matcher;
      std::vector<cv_types::MarkerInfo> markerInfo;
      int numOfObjs;

    public:
      OrbMarkerTracker();
      void registerObject(const cv::Mat frame, cv::Rect roiRect);
      void printRegisteredObjects();
      std::vector<cv_types::MarkerInfo> getRegisteredObjects() { return markerInfo; }

      void setMarkerFromImages(std::string path, std::string stemName, int startIndex, int maxPlaceValue);
      void setMarkerFromImages(std::string path, int startIndex, int maxPlaceValue);
      void setMarkerFromImage(std::string path);
    };

  } // namespace components
} // namespace flexr

#endif

