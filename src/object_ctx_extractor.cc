#include <include/object_ctx_extractor.h>
#include <include/gl_types.h>

namespace mxre
{
  namespace pipeline
  {
    namespace contextualizing
    {
      ObjectCtxExtractor::ObjectCtxExtractor(cv::Mat intrinsic, cv::Mat distCoeffs) :
         camIntrinsic(intrinsic), camDistCoeffs(distCoeffs), raft::kernel()
      {
        input.addPort<cv::Mat>("in_frame");
        input.addPort<std::vector<mxre::cv_units::ObjectInfo>>("in_obj_info");
        input.addPort<clock_t>("in_timestamp");

        // output..
        output.addPort<cv::Mat>("out_frame");
        output.addPort<std::vector<mxre::gltypes::ObjectContext>>("out_obj_context");
        output.addPort<clock_t>("out_timestamp");
      }

      ObjectCtxExtractor::~ObjectCtxExtractor() {}

      raft::kstatus ObjectCtxExtractor::run()
      {
        clock_t rt = clock();

        // get inputs from the previous kernel: ObjectDetector
        auto frame = input["in_frame"].peek<cv::Mat>();
        auto objInfoVec = input["in_obj_info"].peek<std::vector<mxre::cv_units::ObjectInfo>>();
        auto in_st = input["in_timestamp"].peek<clock_t>();
        printf("[ObjectCtxExtractor] ObjDetector->ObjCtxExtractor communication cost %f ms \n",
               ((float)(rt)-in_st) / CLOCKS_PER_SEC * 1000);

        // set outputs
        auto out_frame = output["out_frame"].template allocate_s<cv::Mat>();
        auto out_obj_context =
            output["out_obj_context"].template allocate_s<std::vector<mxre::gltypes::ObjectContext>>();
        auto out_ts = output["out_timestamp"].template allocate_s<clock_t>();

        std::vector<mxre::gltypes::ObjectContext> objCtxVec;
        std::vector<mxre::cv_units::ObjectInfo>::iterator objIter;
        for (objIter = objInfoVec.begin(); objIter != objInfoVec.end(); ++objIter)
        {
          if (objIter->location.size() == 4)
          {
            cv::Mat rvec, tvec;

            std::vector<cv::Point3f> objectPoint3D;
            objectPoint3D.push_back(cv::Point3f(objIter->location[0].x, objIter->location[0].y, 0));
            objectPoint3D.push_back(cv::Point3f(objIter->location[1].x, objIter->location[1].y, 0));
            objectPoint3D.push_back(cv::Point3f(objIter->location[2].x, objIter->location[2].y, 0));
            objectPoint3D.push_back(cv::Point3f(objIter->location[3].x, objIter->location[3].y, 0));
            printf("%f,%f / %f,%f / %f,%f / %f,%f \n\n",
                objIter->location[0].x, objIter->location[0].y,
                objIter->location[1].x, objIter->location[1].y,
                objIter->location[2].x, objIter->location[2].y,
                objIter->location[3].x, objIter->location[3].y);

            cv::solvePnPRansac(objectPoint3D, objIter->roi, camIntrinsic, camDistCoeffs, rvec, tvec);
            cv::Mat rvecMat;
            cv::Rodrigues(rvec, rvecMat);
            cv::Mat cvViewMat(4, 4, CV_64F);
            for (unsigned int row = 0; row < 3; ++row) {
              for (unsigned int col = 0; col < 3; ++col) {
                cvViewMat.at<double>(row, col) = rvecMat.at<double>(row, col);
                printf("%f ", rvecMat.at<double>(row, col));
              }
              printf("\n");
            }
            printf("\n");
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

            for (unsigned int row = 0; row < 4; ++row) {
              for (unsigned int col = 0; col < 4; ++col) {
                objCtx.rotMat[row * 4 + col] = cvViewMat.at<double>(row, col);
                printf("%f ", cvViewMat.at<double>(row, col));
              }
              printf("\n");
            }
            printf("\n");

            objCtx.transVec.x = (tvec.at<float>(0, 0) * 2) / WIDTH * SCALE_FACTOR;
            objCtx.transVec.y = (tvec.at<float>(0, 1) * -2) / WIDTH * SCALE_FACTOR;
            objCtx.transVec.z = -SCALE_FACTOR - (tvec.at<float>(0, 2) / WIDTH * SCALE_FACTOR);
            printf("%f %f %f \n", objCtx.transVec.x, objCtx.transVec.y, objCtx.transVec.z);

            objCtxVec.push_back(objCtx);
          }
        }
        *out_frame = frame;
        *out_obj_context = objCtxVec;
        *out_ts = clock();

        input["in_frame"].recycle();
        input["in_obj_info"].recycle();
        input["in_timestamp"].recycle();

        printf("[ObjectCtxDetector] exe time %f ms \n", ((float)(*out_ts) - in_st) / CLOCKS_PER_SEC * 1000);

        output["out_frame"].send();
        output["out_obj_context"].send();
        output["out_timestamp"].send();
        return raft::proceed;
      }

    } // namespace contextualizing
  }   // namespace pipeline
} // namespace mxre