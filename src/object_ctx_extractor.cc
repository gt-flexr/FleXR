#include <object_ctx_extractor.h>
#include <gl/gl_types.h>

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
        output.addPort<std::vector<mxre::gl::ObjectContext>>("out_obj_context");

#ifdef __PROFILE__
        input.addPort<FrameStamp>("frame_stamp");
        output.addPort<FrameStamp>("frame_stamp");
#endif
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
            output["out_obj_context"].template allocate<std::vector<mxre::gl::ObjectContext>>() );

        std::vector<mxre::gl::ObjectContext> objCtxVec;
        std::vector<mxre::cv_units::ObjectInfo>::iterator objIter;
        for (objIter = objInfoVec.begin(); objIter != objInfoVec.end(); ++objIter)
        {
          if (objIter->location2D.size() == 4 && objIter->isDetected)
          {
            cv::Mat rvec, tvec;

            cv::solvePnPRansac(objIter->rect3D, objIter->location2D, camIntrinsic, camDistCoeffs, rvec, tvec);

            mxre::gl::ObjectContext objCtx;
            objCtx.index = objIter->index;
            float transX = (tvec.at<double>(0, 0) * 2) / WIDTH;
            float transY = (tvec.at<double>(0, 1) * 2) / HEIGHT;
            float transZ = tvec.at<double>(0, 2) / WIDTH;

            float rotX = rvec.at<double>(0, 0);
            float rotY = rvec.at<double>(0, 1);
            float rotZ = rvec.at<double>(0, 2);

            // Convert the OCV coordinate system into the OGL coordinate system
            objCtx.rvec.x = -rotY;   objCtx.rvec.y = -rotZ;   objCtx.rvec.z = rotX;
            objCtx.tvec.x = -transY; objCtx.tvec.y = -transZ; objCtx.tvec.z = transX;

            //def draw_axis(img, R, t, K):
            //  rotV, _ = cv2.Rodrigues(R)
            //    points = np.float32([[100, 0, 0], [0, 100, 0], [0, 0, 100], [0, 0, 0]]).reshape(-1, 3)
            //    axisPoints, _ = cv2.projectPoints(points, rotV, t, K, (0, 0, 0, 0))
            //    img = cv2.line(img, tuple(axisPoints[3].ravel()), tuple(axisPoints[0].ravel()), (255,0,0), 3)
            //    img = cv2.line(img, tuple(axisPoints[3].ravel()), tuple(axisPoints[1].ravel()), (0,255,0), 3)
            //    img = cv2.line(img, tuple(axisPoints[3].ravel()), tuple(axisPoints[2].ravel()), (0,0,255), 3)
            //    return img

            /*
            cv::Mat rvecMat;
            cv::Rodrigues(rvec, rvecMat);
            cv::Mat cvViewMat(4, 4, CV_64F);
            cvViewMat.at<double>(3, 3) = 1.0f;

            // opencv x-y-z == opengl x-z-y
            cv::Mat yzSwapMat = cv::Mat::zeros(4, 4, CV_64F);
            yzSwapMat.at<double>(0, 0) = 1.0f;
            yzSwapMat.at<double>(1, 1) = -1.0f; // Invert the y axis
            yzSwapMat.at<double>(2, 2) = -1.0f; // invert the z axis
            yzSwapMat.at<double>(3, 3) = 1.0f;
            cvViewMat = yzSwapMat * cvViewMat;
            cv::transpose(cvViewMat, cvViewMat); */

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

            // objCtx.modelMat.translate(transX * SCALE_FACTOR, transY * SCALE_FACTOR, -SCALE_FACTOR - transZ * SCALE_FACTOR);

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

        auto &inFrameStamp( input["frame_stamp"].peek<FrameStamp>() );
        auto &outFrameStamp( output["frame_stamp"].allocate<FrameStamp>() );
        outFrameStamp = inFrameStamp;
        input["frame_stamp"].recycle();
        output["frame_stamp"].send();
#endif

        output["out_frame"].send();
        output["out_obj_context"].send();

        return raft::proceed;
      }

    } // namespace contextualizing
  }   // namespace pipeline
} // namespace mxre

