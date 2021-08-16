#include <yaml/kernels/source/yaml_keyboard.h>

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
          if(outPorts[i].connectionType == "local")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsLocal<kernels::KeyboardMsgType>(outPorts[i].portName);
            else
              temp->duplicateOutPortAsLocal<kernels::KeyboardMsgType>(outPorts[i].duplicatedFrom,
                                                                      outPorts[i].portName);
          }
          else if(outPorts[i].connectionType == "remote")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsRemote<kernels::KeyboardMsgType>(
                  outPorts[i].portName, outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
            else
              temp->duplicateOutPortAsRemote<kernels::KeyboardMsgType>(
                  outPorts[i].duplicatedFrom, outPorts[i].portName,
                  outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
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

