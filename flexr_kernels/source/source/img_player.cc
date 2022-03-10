#ifdef __FLEXR_KERNEL_IMG_PLAYER__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>


namespace flexr
{
  namespace kernels
  {
    ImgPlayer::ImgPlayer(std::string id, std::string imgPath, int fnZeros, int fps, int nImgs, bool iterate, bool caching):
      FleXRKernel(id), imgPath(imgPath), fnZeros(fnZeros), nImgs(nImgs), iterate(iterate), caching(caching)
    {
      setName("BagCamera");
      seq = 0;
      freqManager.setFrequency(fps);
      portManager.registerOutPortTag("out_frame", utils::sendLocalFrameCopy, utils::serializeRawFrame);

      imgs.resize(nImgs);
      if(caching)
      {
        for(int i = 0; i < nImgs; i++)
        {
          os << std::setw(fnZeros) << std::setfill('0') << i;
          std::string fn = imgPath + "/" + os.str() + ".png";
          os.str(""); os.clear();
          imgs[i] = cv::imread(fn, cv::IMREAD_UNCHANGED);
        }
      }
    }


    raft::kstatus ImgPlayer::run()
    {
      cv::Mat curImg;
      ImgPlayerMsgType *outFrame = portManager.getOutputPlaceholder<ImgPlayerMsgType>("out_frame");

      double st = getTsNow();

      if(!caching && seq < nImgs)
      {
        os << std::setw(fnZeros) << std::setfill('0') << seq;
        std::string fn = imgPath + "/" + os.str() + ".png";
        os.str(""); os.clear();
        debug_print("File: %s", fn.c_str());
        imgs[seq] = cv::imread(fn, cv::IMREAD_UNCHANGED);
        if(imgs[seq].empty())
        {
          if(iterate)
          {
            debug_print("nImgs is not corresponding to # of imgs: %d/%d... adjusting nImgs and resetting seq", nImgs, seq);
            nImgs = seq;
            seq = 0;
          }
          else
          {
            debug_print("Stop streaming, the actual # of images: %d/%d", nImgs, seq);
            return raft::stop;
          }
        }
      }
      curImg = imgs[seq++];

      outFrame->data = flexr::types::Frame(curImg);
      outFrame->setHeader("ImgPlayer", seq++, getTsNow(), outFrame->data.useAsCVMat().total()*outFrame->data.useAsCVMat().elemSize());
      portManager.sendOutput<ImgPlayerMsgType>("out_frame", outFrame);

      double et = getTsNow();
      if(logger.isSet()) logger.getInstance()->info("{}-th Img\t start\t{}\t end\t{}\t exe\t{}", seq, st, et, et-st);

      if(seq >= nImgs)
      {
        caching = true;
        if(iterate)
        {
          seq = 0;
          debug_print("nImgs, seq: %d/%d, reseting seq...", nImgs, seq);
        }
        else
        {
          debug_print("Stop streaming");
          return raft::stop;
        }
      }

      freqManager.adjust();
      return raft::proceed;
    }
  } // namespace kernels
} // namespace flexr

#endif

