#ifdef __FLEXR_KERNEL_CV2GL_RGB_DISPLAY__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlCv2GlRgbDisplay::YamlCv2GlRgbDisplay(): YamlFleXRKernel()
    {
      width = height = 0;
    }


    void YamlCv2GlRgbDisplay::parseCv2GlRgbDisplay(const YAML::Node &node)
    {
      parseBase(node);
      parseCv2GlRgbDisplaySpecific(node);
    }


    void YamlCv2GlRgbDisplay::parseCv2GlRgbDisplaySpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
    }


    void YamlCv2GlRgbDisplay::printCv2GlRgbDisplay()
    {
      printBase();
      printCv2GlRgbDisplaySpecific();
    }


    void YamlCv2GlRgbDisplay::printCv2GlRgbDisplaySpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tFrame Display Resolution: " << width << " x " << height << std::endl;
    }


    void* YamlCv2GlRgbDisplay::make()
    {
      if(baseSet && specificSet)
      {
        kernels::Cv2GlRgbDisplay *temp = new kernels::Cv2GlRgbDisplay(id, width, height);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);


        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
              temp->activateInPortAsLocal<kernels::Cv2GlRgbDisplayMsgType>(inPorts[i]);
            else
              temp->activateInPortAsRemote<kernels::Cv2GlRgbDisplayMsgType>(inPorts[i]);
          }
          else debug_print("invalid input port_name %s for Cv2GlRgbDisplay", inPorts[i].portName.c_str());

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

