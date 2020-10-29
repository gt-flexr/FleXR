#include <kernels/sources/image_loader.h>
#include <types/cv/types.h>

namespace mxre
{
  namespace kernels
  {
    ImageLoader::ImageLoader(std::string path, std::string stemName, int startIndex, int maxPlaceValue,
        int width, int height): raft::kernel() {
      frame_idx = startIndex;
      this->maxPlaceValue = maxPlaceValue;
      this->path = path;
      this->stemName = stemName;
      this->width = width;
      this->height = height;

      output.addPort<mxre::cv_types::Mat>("out_frame");
#ifdef __PROFILE__
      output.addPort<mxre::types::FrameStamp>("frame_stamp");
#endif

    }


    ImageLoader::~ImageLoader() { }


    raft::kstatus ImageLoader::run() {
      std::stringstream ss;
      ss << std::setfill('0') << std::setw(maxPlaceValue);
      ss << frame_idx++;
      std::string imagePath = path + stemName + ss.str() + ".png";

      auto &frame( output["out_frame"].allocate<mxre::cv_types::Mat>() );
      cv::Mat image = cv::imread(imagePath);

      if(image.empty()) {
        debug_print("Could not read the image: %s", imagePath.c_str());
        return raft::stop;
      }

      int rowPadding = height - image.rows;
      int colPadding = width - image.cols;
      cv::copyMakeBorder(image, image, 0, rowPadding, 0, colPadding, cv::BORDER_CONSTANT, cv::Scalar::all(0));
      //std::string ty =  cv_types::type2str( image.type() );
      //printf("Matrix: %s %dx%d \n", ty.c_str(), image.cols, image.rows );

      frame = mxre::cv_types::Mat(image);
      printf("IMG LOADER %d %d / %d %d\n", frame.cols, frame.rows, frame.cvMat.cols, frame.cvMat.rows);
      output["out_frame"].send();

#ifdef __PROFILE__
      auto &frameStamp( output["frame_stamp"].allocate<mxre::types::FrameStamp>() );
      frameStamp.index = frame_idx;
      frameStamp.st = getNow();
      output["frame_stamp"].send();
#endif

      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

