#include <raft>
#include <mxre.h>
#include <bits/stdc++.h>

using namespace std;


class TestSink : public raft::kernel {
  public:
    TestSink() : raft::kernel() {
      input.addPort<std::vector<mxre::perception::complex_yolo::Prediction>>("in_data");
      input.addPort<mxre::cv_units::Mat>("in_frame");

#ifdef __PROFILE__
      input.addPort<FrameStamp>("frame_stamp");
#endif
    }

    raft::kstatus run() {
      auto &data( input["in_data"].peek<std::vector<mxre::perception::complex_yolo::Prediction>>() );

      for(unsigned int i = 0; i < data.size(); i++) {
        printf("Object: %f %f %f %f %f\n", data[i].cls_pred, data[i].x, data[i].y, data[i].z, data[i].ry);
      }

      auto &frame( input["in_frame"].peek<mxre::cv_units::Mat>() );

      cv::imshow("CVDisplay", frame.cvMat);
      int inKey = cv::waitKey(10) & 0xFF;
      frame.release();

      input["in_frame"].recycle();

#ifdef __PROFILE__
      auto &inFrameStamp( input["frame_stamp"].peek<FrameStamp>() );
      input["frame_stamp"].recycle();
#endif


      input["in_data"].recycle();
      return raft::proceed;
    }
    std::vector<int> test;
};



int main(int argc, char const *argv[])
{
  /*
   * mxre::pipeline::input_srcs::ImageLoader imgLoader(path, common_path); (V)
   * mxre::perception::complex_yolo::PredictionReceiver predictionReceiver(port); (V)
   * mxre::real_context::complex_yolo::ContextExtractor contextExtractor();
   * mxre::pipeline::rendering::ObjectRenderer
   *
   *
   * imagLoader >> ObjectRenderer
   * predictionReceiver >> contextExtractor >> ObjectRenderer
   * ObjectRenderer >> Display
   */

  mxre::pipeline::input_srcs::ImageLoader imgLoader(
      "/home/jin/github/Complex-YOLOv3/data/KITTI/object/training/image_2/", "", 6000, 6, 1252, 380);
  mxre::perception::complex_yolo::PredictionReceiver<mxre::perception::complex_yolo::Prediction> predictionReceiver;
  mxre::pipeline::rendering::ComplexYOLORenderer complexyoloRenderer;
  mxre::pipeline::output_sinks::CVDisplay cvDisplay;
  raft::map pipeline;

  pipeline += imgLoader["out_frame"] >> complexyoloRenderer["in_frame"];
  pipeline += predictionReceiver["out_predictions"] >> complexyoloRenderer["in_obj_context"];
  pipeline += complexyoloRenderer["out_frame"] >> cvDisplay["in_frame"];

#ifdef __PROFILE__
  pipeline += imgLoader["frame_stamp"] >> complexyoloRenderer["frame_stamp"];
  pipeline += complexyoloRenderer["frame_stamp"] >> cvDisplay["frame_stamp"];
#endif

  pipeline.exe();

  /*

  // obj detector - obj ctx extractor
  pipeline += cudaORBDetector["out_frame"] >> objCtxExtractor["in_frame"];
  pipeline += cudaORBDetector["out_obj_info"] >> objCtxExtractor["in_obj_info"];

  // obj ctx extractor - obj renderer
  pipeline += objCtxExtractor["out_frame"] >> objRenderer["in_frame"];
  pipeline += objCtxExtractor["out_obj_context"] >> objRenderer["in_obj_context"];

  // obj renderer - test sink
  pipeline += objRenderer["out_frame"] >> cvDisplay["in_frame"];

#ifdef __PROFILE__
  pipeline += cam["frame_stamp"] >> cudaORBDetector["frame_stamp"];
  pipeline += cudaORBDetector["frame_stamp"] >> objCtxExtractor["frame_stamp"];
  pipeline += objCtxExtractor["frame_stamp"] >> objRenderer["frame_stamp"];
  pipeline += objRenderer["frame_stamp"] >> cvDisplay["frame_stamp"];
#endif

  pipeline.exe();*/
  return 0;
}

