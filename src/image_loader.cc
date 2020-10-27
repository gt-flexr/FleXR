#include "image_loader.h"
#include "cv_types.h"
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace mxre
{
  namespace pipeline
  {
    namespace input_srcs
    {
      ImageLoader::ImageLoader(std::string path, std::string stemName, int startIndex, int maxPlaceValue,
          int width, int height): raft::kernel() {
        frame_idx = startIndex;
        this->maxPlaceValue = maxPlaceValue;
        this->path = path;
        this->stemName = stemName;
        this->width = width;
        this->height = height;

        output.addPort<mxre::cv_units::Mat>("out_frame");
#ifdef __PROFILE__
        output.addPort<FrameStamp>("frame_stamp");
#endif

      }


      ImageLoader::~ImageLoader() { }


      raft::kstatus ImageLoader::run() {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(maxPlaceValue);
        ss << frame_idx++;
        std::string imagePath = path + stemName + ss.str() + ".png";

        auto &frame( output["out_frame"].allocate<mxre::cv_units::Mat>() );
        cv::Mat image = cv::imread(imagePath);

        if(image.empty()) {
          debug_print("Could not read the image: %s", imagePath.c_str());
          return raft::stop;
        }

        int rowPadding = height - image.rows;
        int colPadding = width - image.cols;
        cv::copyMakeBorder(image, image, 0, rowPadding, 0, colPadding, cv::BORDER_CONSTANT, cv::Scalar::all(0));
        //std::string ty =  cv_units::type2str( image.type() );
        //printf("Matrix: %s %dx%d \n", ty.c_str(), image.cols, image.rows );

        frame = mxre::cv_units::Mat(image);
        printf("IMG LOADER %d %d / %d %d\n", frame.cols, frame.rows, frame.cvMat.cols, frame.cvMat.rows);
        output["out_frame"].send();

#ifdef __PROFILE__
        auto &frameStamp( output["frame_stamp"].allocate<FrameStamp>() );
        frameStamp.index = frame_idx;
        frameStamp.st = getNow();
        output["frame_stamp"].send();
#endif

        return raft::proceed;
      }

    } // namespace input_srcs
  }   // namespace pipeline
} // namespace mxre

