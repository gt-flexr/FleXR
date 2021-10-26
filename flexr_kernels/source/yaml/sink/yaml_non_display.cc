#ifdef __FLEXR_KERNEL_NON_DISPLAY__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlNonDisplay::YamlNonDisplay(): YamlFleXRKernel()
    {
      specificSet = true;
    }


    void YamlNonDisplay::parseNonDisplay(const YAML::Node &node)
    {
      parseBase(node);
    }



    void YamlNonDisplay::printNonDisplay()
    {
      printBase();
    }


    void* YamlNonDisplay::make()
    {
      if(baseSet && specificSet)
      {
        kernels::NonDisplay *temp = new kernels::NonDisplay(id);
        temp->setLogger(loggerId, loggerFileName);


        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
              temp->activateInPortAsLocal<kernels::NonDisplayMsgType>(inPorts[i].portName);
            else
              temp->activateInPortAsRemote<kernels::NonDisplayMsgType>(inPorts[i].portName,
                                                                       inPorts[i].protocol,
                                                                       inPorts[i].bindingPortNum);
          }
          else debug_print("invalid input port_name %s for NonDisplay", inPorts[i].portName.c_str());
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

