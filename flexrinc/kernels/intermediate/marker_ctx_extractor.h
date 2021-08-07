#ifndef __FLEXR_OBJ_CTX_EXTRACTOR__
#define __FLEXR_OBJ_CTX_EXTRACTOR__

#include <bits/stdc++.h>
#include <raft>

#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include <glm/glm.hpp>
#include "defs.h"
#include "types/cv/types.h"
#include "types/gl/types.h"
#include "types/clock_types.h"
#include "types/frame.h"
#include "kernels/kernel.h"

namespace flexr
{
  namespace kernels
  {
    using CtxExtractorInMarkerType = types::Message<std::vector<cv_types::DetectedMarker>>;
    using CtxExtractorOutCtxType   = types::Message<std::vector<gl_types::ObjectContext>>;


    /**
     * @brief Kernel to extract the contexts of the detected markers in the corresponding camera frame
     *
     * Port Tag             | Type
     * ---------------------| ----------------------------
     * in_detected_markers  | @ref flexr::types::Message<std::vector<@ref flexr::cv_types::DetectedMarker>>
     * out_marker_contexts  | @ref flexr::types::Message<std::vector<@ref flexr::gl_types::ObjectContext>>
    */
    class MarkerCtxExtractor : public FleXRKernel
    {
    private:
      cv::Mat camIntrinsic;
      cv::Mat camDistCoeffs;
      int width, height;


    public:
      /**
       * @brief Initialize kernel with frame info
       * @param width
       *  Frame width
       * @param height
       *  Frame height
       */
      MarkerCtxExtractor(int width=1280, int height=720);


      /**
       * @brief Initialize kernel with frame and camera info
       * @param width
       *  Frame width
       * @param height
       *  Frame height
       * @param inIntrinsic
       *  3x3 camera intrinsic matrix to set
       * @param inDistCoeffs
       *  4x1 distortion coefficient matrix to set
       */
      MarkerCtxExtractor(int width, int height, cv::Mat intrinsic, cv::Mat distCoeffs);


      /**
       * @brief Set camera intrinsic matrix
       * @param inIntrinsic
       *  3x3 camera intrinsic matrix to set
       */
      void setIntrinsic(cv::Mat inIntrinsic) {camIntrinsic = inIntrinsic.clone();}


      /**
       * @brief Set camera distortion coefficients
       * @param inDistCoeffs
       *  4x1 distortion coefficient matrix to set
       */
      void setDistCoeffs(cv::Mat inDistCoeffs) {camDistCoeffs = inDistCoeffs.clone();}


      virtual raft::kstatus run();


      bool logic(CtxExtractorInMarkerType *inDetectedMarkers, CtxExtractorOutCtxType *outMarkerContexts);
    };

  }   // namespace pipeline
} // namespace flexr

#endif

