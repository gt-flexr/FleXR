#include <kernels/intermediate/marker_ctx_extractor.h>
#include <unistd.h>

namespace mxre
{
  namespace kernels
  {
    MarkerCtxExtractor::MarkerCtxExtractor(int width, int height) {
      cv::Mat tempIntrinsic(3, 3, CV_64FC1);
      cv::Mat tempDistCoeffs(4, 1, CV_64FC1, {0, 0, 0, 0});

      tempIntrinsic.at<double>(0, 0) = width;
      tempIntrinsic.at<double>(0, 1) = 0;
      tempIntrinsic.at<double>(0, 2) = width/2;
      tempIntrinsic.at<double>(1, 0) = 0;
      tempIntrinsic.at<double>(1, 1) = width;
      tempIntrinsic.at<double>(1, 2) = height/2;
      tempIntrinsic.at<double>(2, 0) = 0;
      tempIntrinsic.at<double>(2, 1) = 0;
      tempIntrinsic.at<double>(2, 2) = 1;

      this->width = width;
      this->height = height;
      camIntrinsic = tempIntrinsic.clone();
      camDistCoeffs = tempDistCoeffs.clone();

      addInputPort<types::Message<std::vector<cv_types::DetectedMarker>>>("in_detected_markers");
      addOutputPort<types::Message<std::vector<gl_types::ObjectContext>>>("out_marker_contexts");

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("marker_ctx_extractor", "logs/marker_ctx_extractor.log");
#endif
    }


    MarkerCtxExtractor::MarkerCtxExtractor(int width, int height, cv::Mat intrinsic, cv::Mat distCoeffs) :
      MXREKernel()
    {
      this->width = width;
      this->height = height;
      camIntrinsic = intrinsic.clone();
      camDistCoeffs = distCoeffs.clone();

      addInputPort<types::Message<std::vector<cv_types::DetectedMarker>>>("in_detected_markers");
      addOutputPort<types::Message<std::vector<gl_types::ObjectContext>>>("out_marker_contexts");

#ifdef __PROFILE__
      if(logger == NULL) initLoggerST("marker_ctx_extractor", "logs/marker_ctx_extractor.log");
#endif

    }


    bool MarkerCtxExtractor::logic(types::Message<std::vector<cv_types::DetectedMarker>> &inDetectedMarkers,
                                   types::Message<std::vector<gl_types::ObjectContext>> &outMarkerContexts)
    {
      outMarkerContexts.tag = inDetectedMarkers.tag;
      outMarkerContexts.seq = inDetectedMarkers.seq;
      outMarkerContexts.ts  = inDetectedMarkers.ts;

      std::vector<mxre::cv_types::DetectedMarker>::iterator detectedMarker;
      for (detectedMarker = inDetectedMarkers.data.begin();
           detectedMarker != inDetectedMarkers.data.end();
           ++detectedMarker)
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
        outMarkerContexts.data.push_back(markerContext);
      }
      return true;
    }


    raft::kstatus MarkerCtxExtractor::run()
    {
      types::Message<std::vector<cv_types::DetectedMarker>> &inDetectedMarkers = \
                            input["in_detected_markers"].peek<types::Message<std::vector<cv_types::DetectedMarker>>>();
      types::Message<std::vector<gl_types::ObjectContext>> &outMarkerContexts = \
          output["out_marker_contexts"].template allocate<types::Message<std::vector<gl_types::ObjectContext>>>();
#ifdef __PROFILE__
      startTimeStamp = getTimeStampNow();
#endif

      if(logic(inDetectedMarkers, outMarkerContexts)) {
        output["out_marker_contexts"].send();
        sendPrimitiveCopy("out_marker_contexts", outMarkerContexts);
      }

      recyclePort("in_detected_markers");

      debug_print("END");
#ifdef __PROFILE__
      endTimeStamp = getTimeStampNow();
      logger->info("{}\t {}\t {}", startTimeStamp, endTimeStamp, endTimeStamp-startTimeStamp);
#endif

      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

