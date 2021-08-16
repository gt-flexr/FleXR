#include <yaml/kernels/sink/yaml_cv_display.h>

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
          if(inPorts[i].connectionType == "local")
            temp->activateInPortAsLocal<kernels::CVDisplayMsgType>(inPorts[i].portName);
          else
            temp->activateInPortAsRemote<kernels::CVDisplayMsgType>(inPorts[i].portName, inPorts[i].bindingPortNum);
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

