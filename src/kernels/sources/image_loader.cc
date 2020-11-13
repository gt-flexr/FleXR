#include <kernels/sources/image_loader.h>
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

      addOutputPort<mxre::types::Frame>("out_frame");
    }


    /* Destructor */
    ImageLoader::~ImageLoader() { }


    /* Kernel Run */
    raft::kstatus ImageLoader::run() {
#ifdef __PROFILE__
      start = getNow();
#endif

      std::stringstream ss;
      ss << std::setfill('0') << std::setw(maxPlaceValue);
      ss << frame_idx++;
      std::string imagePath = path + stemName + ss.str() + ".png";

      auto &outFrame( output["out_frame"].allocate<mxre::types::Frame>() );
      outFrame = mxre::types::Frame(height, width, CV_8UC3);
      cv::Mat outFrameAsCVMat = outFrame.useAsCVMat();
      outFrameAsCVMat = cv::imread(imagePath);
      if(outFrameAsCVMat.empty()) {
        debug_print("Could not read the image: %s", imagePath.c_str());
        return raft::stop;
      }

      int rowPadding = height - outFrameAsCVMat.rows;
      int colPadding = width - outFrameAsCVMat.cols;
      cv::copyMakeBorder(outFrameAsCVMat, outFrameAsCVMat, 0, rowPadding, 0, colPadding, cv::BORDER_CONSTANT,
          cv::Scalar::all(0));
      //std::string ty =  cv_types::type2str( image.type() );
      //printf("Matrix: %s %dx%d \n", ty.c_str(), image.cols, image.rows );

      printf("IMG LOADER %d %d / %d %d\n", (int)outFrame.cols, (int)outFrame.rows,
             outFrameAsCVMat.cols, outFrameAsCVMat.rows);
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

