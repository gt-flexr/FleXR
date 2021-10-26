#ifdef __FLEXR_KERNEL_CV_DISPLAY__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlCvDisplay::YamlCvDisplay(): YamlFleXRKernel()
    {
      specificSet = true;
    }


    void YamlCvDisplay::parseCvDisplay(const YAML::Node &node)
    {
      parseBase(node);
    }



    void YamlCvDisplay::printCvDisplay()
    {
      printBase();
    }


    void* YamlCvDisplay::make()
    {
      if(baseSet && specificSet)
      {
        kernels::CVDisplay *temp = new kernels::CVDisplay(id);
        temp->setLogger(loggerId, loggerFileName);


        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
              temp->activateInPortAsLocal<kernels::CVDisplayMsgType>(inPorts[i].portName);
            else
              temp->activateInPortAsRemote<kernels::CVDisplayMsgType>(inPorts[i].portName,
                                                                      inPorts[i].protocol,
                                                                      inPorts[i].bindingPortNum);
          }
          else debug_print("invalid input port_name %s for CVDisplay", inPorts[i].portName.c_str());

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

