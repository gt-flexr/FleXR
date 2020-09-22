#include <include/object_ctx_extractor.h>
#include <include/gl_types.h>

namespace mxre
{
  namespace pipeline
  {
    namespace contextualizing
    {
      ObjectCtxExtractor::ObjectCtxExtractor(cv::Mat intrinsic, cv::Mat distCoeffs) : raft::kernel()
      {
        camIntrinsic = intrinsic.clone();
        camDistCoeffs = distCoeffs.clone();

        input.addPort<mxre::cv_units::Mat>("in_frame");
        input.addPort<std::vector<mxre::cv_units::ObjectInfo>>("in_obj_info");

        // output..
        output.addPort<mxre::cv_units::Mat>("out_frame");
        output.addPort<std::vector<mxre::gltypes::ObjectContext>>("out_obj_context");
      }

      ObjectCtxExtractor::~ObjectCtxExtractor() {}

      raft::kstatus ObjectCtxExtractor::run()
      {

#ifdef __PROFILE__
        TimeVal start = getNow();
#endif

        // get inputs from the previous kernel: ObjectDetector
        auto &frame( input["in_frame"].peek<mxre::cv_units::Mat>() );
        auto &objInfoVec( input["in_obj_info"].peek<std::vector<mxre::cv_units::ObjectInfo>>() );

        // set outputs
        auto &out_frame( output["out_frame"].template allocate<mxre::cv_units::Mat>() );
        auto &out_obj_context(
            output["out_obj_context"].template allocate<std::vector<mxre::gltypes::ObjectContext>>() );

        std::vector<mxre::gltypes::ObjectContext> objCtxVec;
        std::vector<mxre::cv_units::ObjectInfo>::iterator objIter;
        for (objIter = objInfoVec.begin(); objIter != objInfoVec.end(); ++objIter)
        {
          if (objIter->location2D.size() == 4 && objIter->isDetected)
          {
            cv::Mat rvec, tvec;
            //printf("camIntrinsic %f %f %f \n", camIntrinsic.at<double>(0, 0), camIntrinsic.at<double>(0, 1),
            //    camIntrinsic.at<double>(0, 2));

            cv::solvePnPRansac(objIter->rect3D, objIter->location2D, camIntrinsic, camDistCoeffs, rvec, tvec);
            cv::Mat rvecMat;
            cv::Rodrigues(rvec, rvecMat);
            cv::Mat cvViewMat(4, 4, CV_64F);
            /*
            for (unsigned int row = 0; row < 3; ++row) {
              for (unsigned int col = 0; col < 3; ++col) {
                cvViewMat.at<double>(row, col) = rvecMat.at<double>(row, col);
                printf("%f ", rvecMat.at<double>(row, col));
              }
              printf("\n");
            }
            printf("\n");
            */
            cvViewMat.at<double>(3, 3) = 1.0f;

            // opencv x-y-z == opengl x-z-y
            cv::Mat yzSwapMat = cv::Mat::zeros(4, 4, CV_64F);
            yzSwapMat.at<double>(0, 0) = 1.0f;
            yzSwapMat.at<double>(1, 1) = -1.0f; // Invert the y axis
            yzSwapMat.at<double>(2, 2) = -1.0f; // invert the z axis
            yzSwapMat.at<double>(3, 3) = 1.0f;
            cvViewMat = yzSwapMat * cvViewMat;
            cv::transpose(cvViewMat, cvViewMat);

            mxre::gltypes::ObjectContext objCtx;
            objCtx.index = objIter->index;

            /*
            for (unsigned int row = 0; row < 4; ++row) {
              for (unsigned int col = 0; col < 4; ++col) {
                objCtx.modelMat[row * 4 + col] = cvViewMat.at<double>(row, col);
                printf("%f ", cvViewMat.at<double>(row, col));
              }
              printf("\n");
            }
            printf("\n");
            */

            float transX = (tvec.at<double>(0, 0) * 2) / WIDTH;
            float transY = (tvec.at<double>(0, 1) * 2) / WIDTH;
            float transZ = tvec.at<double>(0, 2) / WIDTH;
            //printf("[Obj Position] x(%lf), y(%lf), z(%lf) \n", transX, transY, transZ);
            //printf("[Obj Position with SCALE_FACTOR] x(%lf), y(%lf), z(%lf) \n",
            //       transX * SCALE_FACTOR, transY * SCALE_FACTOR, -SCALE_FACTOR - transZ * SCALE_FACTOR);
            //printf("\n");
            objCtx.modelMat.translate(transX * SCALE_FACTOR, transY * SCALE_FACTOR, -SCALE_FACTOR - transZ * SCALE_FACTOR);

            objCtxVec.push_back(objCtx);
          }
        }
        out_frame = frame;
        out_obj_context = objCtxVec;

        input["in_frame"].recycle();
        input["in_obj_info"].recycle();

#ifdef __PROFILE__
        TimeVal end = getNow();
        debug_print("Exe Time: %lfms", getExeTime(end, start));
#endif

        output["out_frame"].send();
        output["out_obj_context"].send();

        return raft::proceed;
      }

    } // namespace contextualizing
  }   // namespace pipeline
} // namespace mxre

