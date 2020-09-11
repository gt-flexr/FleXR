#include <include/cv_display.h>
#include <include/cv_types.h>

namespace mxre
{
  namespace pipeline
  {
    namespace output_sinks
    {
      CVDisplay::CVDisplay()
      {
        input.addPort<cv::Mat>("in_frame");
      }

      CVDisplay::~CVDisplay() {}

      raft::kstatus CVDisplay::run()
      {
        auto frame = input["in_frame"].peek<cv::Mat>();

        cv::imshow("CVDisplay", frame);
        int inKey = cv::waitKey(100) & 0xFF;

        cv_units::releaseMat(frame);
        input["in_frame"].recycle();

        return raft::proceed;
      }
    } // namespace input_srcs
  }   // namespace pipeline
} // namespace mxre

