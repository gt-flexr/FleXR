#include <raft>
#include <mxre>
#include <bits/stdc++.h>

using namespace std;


class TestSink : public mxre::kernels::MXREKernel {
  public:
    TestSink() : MXREKernel() {
      addInputPort<std::vector<mxre::types::ComplexYoloPrediction>>("in_data");
      addInputPort<mxre::types::Frame>("in_frame");
    }

    raft::kstatus run() {
      auto &data( input["in_data"].peek<std::vector<mxre::types::ComplexYoloPrediction>>() );

      for(unsigned int i = 0; i < data.size(); i++) {
        printf("Object: %f %f %f %f %f\n", data[i].cls_pred, data[i].x, data[i].y, data[i].z, data[i].ry);
      }

      auto &frame( input["in_frame"].peek<mxre::types::Frame>() );

      cv::imshow("CVDisplay", frame.useAsCVMat());
      int inKey = cv::waitKey(10) & 0xFF;
      frame.release();

      recyclePort("in_frame");
      recyclePort("in_data");
      return raft::proceed;
    }
    std::vector<int> test;
};



int main(int argc, char const *argv[])
{
  /*
   * mxre::pipeline::input_srcs::ImageLoader imgLoader(path, common_path); (V)
   * mxre::types::ComplexYoloPredictionReceiver predictionReceiver(port); (V)
   * mxre::real_context::complex_yolo::ContextExtractor contextExtractor();
   * mxre::pipeline::rendering::ObjectRenderer
   *
   *
   * imagLoader >> ObjectRenderer
   * predictionReceiver >> contextExtractor >> ObjectRenderer
   * ObjectRenderer >> Display
   */

  mxre::kernels::ImageLoader imgLoader(
      "/home/jin/github/Complex-YOLOv3/data/KITTI/object/training/image_2/", "", 6000, 6, 1252, 380);
  mxre::kernels::PredictionReceiver<mxre::types::ComplexYoloPrediction> predictionReceiver;
  mxre::kernels::ComplexYOLORenderer complexyoloRenderer;
  mxre::kernels::CVDisplay cvDisplay;
  raft::map pipeline;

  pipeline += imgLoader["out_frame"] >> complexyoloRenderer["in_frame"];
  pipeline += predictionReceiver["out_predictions"] >> complexyoloRenderer["in_obj_context"];
  pipeline += complexyoloRenderer["out_frame"] >> cvDisplay["in_frame"];

#ifdef __PROFILE__
  pipeline += imgLoader["frame_stamp"] >> complexyoloRenderer["frame_stamp"];
  pipeline += complexyoloRenderer["frame_stamp"] >> cvDisplay["frame_stamp"];
#endif

  pipeline.exe();

  return 0;
}

