#ifdef __FLEXR_KERNEL_KEYBOARD__

#include <flexr_kernels/include/kernels.h>

namespace flexr
{
  namespace yaml
  {

    YamlKeyboard::YamlKeyboard(): YamlFleXRKernel()
    {
      specificSet = true;
    }


    void YamlKeyboard::parseKeyboard(const YAML::Node &node)
    {
      parseBase(node);
    }


    void YamlKeyboard::printKeyboard()
    {
      printBase();
    }


    void* YamlKeyboard::make()
    {
      if(baseSet && specificSet)
      {
        kernels::Keyboard *temp = new kernels::Keyboard(id, frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_key")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::KeyboardMsgType>(outPorts[i].portName);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::KeyboardMsgType>(outPorts[i].portName,
                                                                      outPorts[i].protocol,
                                                                      outPorts[i].connectingAddr,
                                                                      outPorts[i].connectingPortNum);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_key")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::KeyboardMsgType>(outPorts[i].duplicatedFrom,
                                                                        outPorts[i].portName);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::KeyboardMsgType>(
                    outPorts[i].duplicatedFrom, outPorts[i].portName,
                    outPorts[i].protocol, outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
            }
            else debug_print("invalid output port_name %s for Keyboard", outPorts[i].portName.c_str());
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

