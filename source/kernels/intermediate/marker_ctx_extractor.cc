#include <kernels/intermediate/marker_ctx_extractor.h>
#include <utils/msg_receiving_functions.h>
#include <utils/msg_sending_functions.h>
#include <unistd.h>

namespace flexr
{
  namespace kernels
  {
    MarkerCtxExtractor::MarkerCtxExtractor(int width, int height) {
      portManager.registerInPortTag("in_detected_markers",
                                    components::PortDependency::BLOCKING,
                                    utils::recvDetectedMarkers);
      portManager.registerOutPortTag("out_marker_contexts",
                                     utils::sendLocalBasicCopy<CtxExtractorOutCtxType>,
                                     utils::sendRemotePrimitiveVec<CtxExtractorOutCtxType>,
                                     types::freePrimitiveMsg<CtxExtractorOutCtxType>);

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
    }


    MarkerCtxExtractor::MarkerCtxExtractor(int width, int height, cv::Mat intrinsic, cv::Mat distCoeffs) :
      FleXRKernel()
    {
      this->width = width;
      this->height = height;
      camIntrinsic = intrinsic.clone();
      camDistCoeffs = distCoeffs.clone();

      portManager.registerInPortTag("in_detected_markers",
                                    components::PortDependency::BLOCKING,
                                    utils::recvDetectedMarkers);
      portManager.registerOutPortTag("out_marker_contexts",
                                     utils::sendLocalBasicCopy<CtxExtractorOutCtxType>,
                                     utils::sendRemotePrimitiveVec<CtxExtractorOutCtxType>,
                                     types::freePrimitiveMsg<CtxExtractorOutCtxType>);
    }


    bool MarkerCtxExtractor::logic(CtxExtractorInMarkerType *inDetectedMarkers,
                                   CtxExtractorOutCtxType   *outMarkerContexts)
    {
      strcpy(outMarkerContexts->tag, "marker_ctx");
      outMarkerContexts->seq = inDetectedMarkers->seq;
      outMarkerContexts->ts  = inDetectedMarkers->ts;

      std::vector<flexr::cv_types::DetectedMarker>::iterator detectedMarker;
      for (detectedMarker = inDetectedMarkers->data.begin();
           detectedMarker != inDetectedMarkers->data.end();
           ++detectedMarker)
      {
        cv::Mat rvec, tvec;

        cv::solvePnPRansac(detectedMarker->defaultLocationIn3D, detectedMarker->locationIn2D,
                           camIntrinsic, camDistCoeffs, rvec, tvec);

        flexr::gl_types::ObjectContext markerContext;
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
        outMarkerContexts->data.push_back(markerContext);
      }
      return true;
    }


    raft::kstatus MarkerCtxExtractor::run()
    {
      CtxExtractorInMarkerType *inDetectedMarkers = \
                                                portManager.getInput<CtxExtractorInMarkerType>("in_detected_markers");
      CtxExtractorOutCtxType *outMarkerContexts = \
                                       portManager.getOutputPlaceholder<CtxExtractorOutCtxType>("out_marker_contexts");

      double st = getTsNow();
      logic(inDetectedMarkers, outMarkerContexts);
      portManager.sendOutput("out_marker_contexts", outMarkerContexts);
      double et = getTsNow();

      portManager.freeInput("in_detected_markers", inDetectedMarkers);

      debug_print("st(%lf) et(%lf) exe(%lf)", st, et, et-st);
      if(logger.isSet()) logger.getInstance()->info("{}\t {}\t {}", st, et, et-st);

      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

