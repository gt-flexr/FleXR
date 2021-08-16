#include <yaml/kernels/sink/yaml_non_display.h>

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
          if(inPorts[i].connectionType == "local")
            temp->activateInPortAsLocal<kernels::NonDisplayMsgType>(inPorts[i].portName);
          else
            temp->activateInPortAsRemote<kernels::NonDisplayMsgType>(inPorts[i].portName, inPorts[i].bindingPortNum);
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

