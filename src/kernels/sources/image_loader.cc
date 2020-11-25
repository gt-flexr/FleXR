#include <kernels/sources/image_loader.h>
#include <opencv2/highgui.hpp>
#include <types/cv/types.h>

namespace mxre
{
  namespace kernels
  {
    /* Constructor */
    ImageLoader::ImageLoader(std::string path, std::string stemName, int startIndex, int maxPlaceValue,
        int width, int height): MXREKernel() {
      frame_idx = startIndex;
      this->maxPlaceValue = maxPlaceValue;
      this->path = path;
      this->stemName = stemName;
      this->width = width;
      this->height = height;
      this->periodMS = 0;

      addOutputPort<mxre::types::Frame>("out_frame");
    }


    /* Destructor */
    ImageLoader::~ImageLoader() { }


    /* Kernel Run */
    raft::kstatus ImageLoader::run() {
      sleepForMS(periodMS); // control read frequency

#ifdef __PROFILE__
      start = getNow();
#endif

      std::stringstream ss;
      ss << std::setfill('0') << std::setw(maxPlaceValue);
      ss << frame_idx++;
      std::string imagePath = path + stemName + ss.str() + ".png";

      auto &outFrame( output["out_frame"].allocate<mxre::types::Frame>() );
      cv::Mat image = cv::imread(imagePath);
      if(image.empty()) {
        debug_print("Could not read the image: %s", imagePath.c_str());
        return raft::stop;
      }

      int rowPadding = height - image.rows;
      int colPadding = width - image.cols;
      if(rowPadding > 0 && colPadding > 0) {
        //debug_print("padding : %d %d", rowPadding, colPadding);
        cv::copyMakeBorder(image, image, 0, rowPadding, 0, colPadding, cv::BORDER_CONSTANT,
            cv::Scalar::all(0));
      }

      outFrame = mxre::types::Frame(image);
      //debug_print("IMG LOADER %d %d from image %d %d\n", (int)outFrame.cols, (int)outFrame.rows, image.cols, image.rows);
      output["out_frame"].send();
      sendFrameCopy("out_frame", &outFrame);

#ifdef __PROFILE__
      end = getNow();
      profile_print("Exe Time: %lf ms", getExeTime(end, start));
#endif

      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

