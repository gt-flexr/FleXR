#include <kernels/perceptions/orb/marker_ctx_extractor.h>

namespace mxre
{
  namespace kernels
  {
    MarkerCtxExtractor::MarkerCtxExtractor(int width, int height): MXREKernel(), width(width), height(height) {
      camIntrinsic = cv::Mat(3, 3, CV_64FC1);
      camDistCoeffs = cv::Mat(4, 1, CV_64FC1, {0, 0, 0, 0});

      camIntrinsic.at<double>(0, 0) = width;
      camIntrinsic.at<double>(0, 1) = 0;
      camIntrinsic.at<double>(0, 2) = width/2;
      camIntrinsic.at<double>(1, 0) = 0;
      camIntrinsic.at<double>(1, 1) = width;
      camIntrinsic.at<double>(1, 2) = height/2;
      camIntrinsic.at<double>(2, 0) = 0;
      camIntrinsic.at<double>(2, 1) = 0;
      camIntrinsic.at<double>(2, 2) = 1;

      addInputPort<std::vector<mxre::cv_types::DetectedMarker>>("in_detected_markers");
      addOutputPort<std::vector<mxre::gl_types::ObjectContext>>("out_marker_contexts");
    }


    MarkerCtxExtractor::MarkerCtxExtractor(int width, int height, cv::Mat intrinsic, cv::Mat distCoeffs) :
      MXREKernel(), width(width), height(height)
    {
      camIntrinsic = intrinsic.clone();
      camDistCoeffs = distCoeffs.clone();

      addInputPort<std::vector<mxre::cv_types::DetectedMarker>>("in_detected_markers");
      addOutputPort<std::vector<mxre::gl_types::ObjectContext>>("out_marker_contexts");
    }


    bool MarkerCtxExtractor::logic(std::vector<mxre::cv_types::DetectedMarker> *inDetectedMarkers,
               std::vector<mxre::gl_types::ObjectContext> *outMarkerContexts)
    {
      std::vector<mxre::cv_types::DetectedMarker>::iterator detectedMarker;
      for (detectedMarker = inDetectedMarkers->begin(); detectedMarker != inDetectedMarkers->end(); ++detectedMarker)
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

        // Convert the OCV coordinate system into the OGL coordinate system
        //markerContext.rvec.x = -rotY;   markerContext.rvec.y = -rotZ;   markerContext.rvec.z = rotX;
        //markerContext.tvec.x = transY; markerContext.tvec.y = -transZ; markerContext.tvec.z = transX;
        markerContext.rvec.x = rotX;   markerContext.rvec.y = -rotY;   markerContext.rvec.z = -rotZ;
        markerContext.tvec.x = transX; markerContext.tvec.y = transY; markerContext.tvec.z = -transZ;
        outMarkerContexts->push_back(markerContext);
      }
      return true;
    }


    raft::kstatus MarkerCtxExtractor::run()
    {

#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif
      debug_print("START");
      auto &inDetectedMarkers(input["in_detected_markers"].peek<std::vector<mxre::cv_types::DetectedMarker>>() );
      auto &outMarkerContexts(
          output["out_marker_contexts"].template allocate<std::vector<mxre::gl_types::ObjectContext>>());

      if(logic(&inDetectedMarkers, &outMarkerContexts)) {
        output["out_marker_contexts"].send();
        sendPrimitiveCopy("out_marker_contexts", &outMarkerContexts);
      }

      recyclePort("in_detected_markers");

      debug_print("END");
#ifdef __PROFILE__
      mxre::types::TimeVal end = getNow();
      profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

