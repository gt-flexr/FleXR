#include <kernels/perceptions/orb/object_ctx_extractor.h>

namespace mxre
{
  namespace kernels
  {
    ObjectCtxExtractor::ObjectCtxExtractor(int width, int height): MXREKernel(), width(width), height(height) {
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

      addInputPort<std::vector<mxre::cv_types::ObjectInfo>>("in_obj_info");
      addOutputPort<std::vector<mxre::gl_types::ObjectContext>>("out_obj_context");
    }


    ObjectCtxExtractor::ObjectCtxExtractor(int width, int height, cv::Mat intrinsic, cv::Mat distCoeffs) :
      MXREKernel(), width(width), height(height)
    {
      camIntrinsic = intrinsic.clone();
      camDistCoeffs = distCoeffs.clone();

      addInputPort<std::vector<mxre::cv_types::ObjectInfo>>("in_obj_info");
      addOutputPort<std::vector<mxre::gl_types::ObjectContext>>("out_obj_context");
    }


    bool ObjectCtxExtractor::logic(std::vector<mxre::cv_types::ObjectInfo> *inObjInfo,
               std::vector<mxre::gl_types::ObjectContext> *outObjContext)
    {
      std::vector<mxre::cv_types::ObjectInfo>::iterator objIter;
      for (objIter = inObjInfo->begin(); objIter != inObjInfo->end(); ++objIter)
      {
        if (objIter->location2D.size() == 4 && objIter->isDetected)
        {
          cv::Mat rvec, tvec;

          cv::solvePnPRansac(objIter->rect3D, objIter->location2D, camIntrinsic, camDistCoeffs, rvec, tvec);

          mxre::gl_types::ObjectContext objCtx;
          objCtx.index = objIter->index;
          float transX = (tvec.at<double>(0, 0) * 2) / width;
          float transY = (tvec.at<double>(0, 1) * 2) / height;
          float transZ = tvec.at<double>(0, 2) / width;

          float rotX = rvec.at<double>(0, 0);
          float rotY = rvec.at<double>(0, 1);
          float rotZ = rvec.at<double>(0, 2);

          // Convert the OCV coordinate system into the OGL coordinate system
          //objCtx.rvec.x = -rotY;   objCtx.rvec.y = -rotZ;   objCtx.rvec.z = rotX;
          //objCtx.tvec.x = transY; objCtx.tvec.y = -transZ; objCtx.tvec.z = transX;
          objCtx.rvec.x = rotX;   objCtx.rvec.y = -rotY;   objCtx.rvec.z = -rotZ;
          objCtx.tvec.x = transX; objCtx.tvec.y = transY; objCtx.tvec.z = -transZ;
          outObjContext->push_back(objCtx);
        }
      }

      return true;
    }


    raft::kstatus ObjectCtxExtractor::run()
    {

#ifdef __PROFILE__
      mxre::types::TimeVal start = getNow();
#endif
      debug_print("START");
      auto &inObjInfo( input["in_obj_info"].peek<std::vector<mxre::cv_types::ObjectInfo>>() );
      auto &outObjContext( output["out_obj_context"].template allocate<std::vector<mxre::gl_types::ObjectContext>>() );

      if(logic(&inObjInfo, &outObjContext)) {
        output["out_obj_context"].send();
        sendPrimitiveCopy("out_obj_context", &outObjContext);
      }

      recyclePort("in_obj_info");

      debug_print("END");
#ifdef __PROFILE__
      mxre::types::TimeVal end = getNow();
      profile_print("Exe Time: %lfms", getExeTime(end, start));
#endif

      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

