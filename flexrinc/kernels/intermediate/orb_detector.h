#ifndef __FLEXR_OBJ_DETECTOR__
#define __FLEXR_OBJ_DETECTOR__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"

#include "types/cv/types.h"
#include "types/clock_types.h"
#include "types/frame.h"
#include "utils/cv_utils.h"

#include "components/orb_marker_tracker.h"

#include "kernels/kernel.h"

namespace flexr
{
  namespace kernels
  {

    using ORBDetectorInFrameType   = types::Message<types::Frame>;
    using ORBDetectorOutMarkerType = types::Message<std::vector<cv_types::DetectedMarker>>;


    /**
     * @brief Kernel to detect an registered marker with local feature matching and ORB algorithm
     *
     * Port Tag             | Type
     * ---------------------| ----------------------------
     * in_frame             | @ref flexr::types::Message<@ref flexr::types::Frame>
     * out_detected_markers | @ref flexr::types::Message<std::vector<@ref flexr::cv_types::DetectedMarker>>
     */
    class ORBDetector : public FleXRKernel
    {
      private:
        components::OrbMarkerTracker orbMarkerTracker;
        std::vector<flexr::cv_types::MarkerInfo> registeredMarkers;
        cv::Ptr<cv::Feature2D> detector;
        cv::Ptr<cv::DescriptorMatcher> matcher;

        double knnMatchRatio;
        int knnParam;
        double ransacThresh;
        int minInlierThresh;


      public:
        /**
         * @brief Initialize kernel with registered marker info
         * @param id
         *  Kernel ID
         * @param markerImage
         *  Marker image file to set it as a marker
         */
        ORBDetector(std::string id, std::string markerImage);

        raft::kstatus run() override;

        bool logic(ORBDetectorInFrameType   *inFrame,
                   ORBDetectorOutMarkerType *outDetectedMarkers);
    };

  }   // namespace kernels
} // namespace flexr

#endif

