#ifdef __FLEXR_KERNEL_IMG_PLAYER__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlImgPlayer::YamlImgPlayer(): YamlFleXRKernel()
    {
      imgPath = "";
      fnZeros = nImgs = 0;
      iterate = caching = false;
    }


    void YamlImgPlayer::parseImgPlayer(const YAML::Node &node)
    {
      parseBase(node);
      parseImgPlayerSpecific(node);
    }


    void YamlImgPlayer::parseImgPlayerSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      imgPath           = others["image_path"].as<std::string>();
      fnZeros           = others["filename_zeros"].as<int>();
      nImgs             = others["num_of_images"].as<int>();
      iterate           = others["iterate"].as<bool>();
      caching           = others["caching"].as<bool>();
    }


    void YamlImgPlayer::printImgPlayer()
    {
      printBase();
      printImgPlayerSpecific();
    }


    void YamlImgPlayer::printImgPlayerSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tImage path: " << imgPath << std::endl;
      std::cout << "\tFielname Zeros: " << fnZeros << ", Total # of images: " << nImgs << std::endl;
      std::cout << "\tIterate: " << iterate << ", Initial caching: " << caching << std::endl;
    }


    void* YamlImgPlayer::make()
    {
      if(baseSet && specificSet)
      {
        kernels::ImgPlayer *temp = new kernels::ImgPlayer(id, imgPath, fnZeros, frequency, nImgs, iterate, caching);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_frame")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::ImgPlayerMsgType>(outPorts[i]);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::ImgPlayerMsgType>(outPorts[i]);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_frame")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::ImgPlayerMsgType>(outPorts[i]);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::ImgPlayerMsgType>(outPorts[i]);
            }
            else debug_print("invalid output port_name %s for ImgPlayer", outPorts[i].portName.c_str());
          }
        }
        return temp;
      }
      else
      {
        debug_print("yaml recipe is invalid, fail to make.");
      }

      return nullptr;
    }

  }
}

#endif

