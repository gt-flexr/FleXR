#include <yaml/kernels/intermediate/yaml_marker_ctx_extractor.h>

namespace flexr
{
  namespace yaml
  {

    YamlMarkerCtxExtractor::YamlMarkerCtxExtractor(): YamlFleXRKernel()
    {
      width = height = 0;
    }


    void YamlMarkerCtxExtractor::parseMarkerCtxExtractor(const YAML::Node &node)
    {
      parseBase(node);
      parseMarkerCtxExtractorSpecific(node);
    }


    void YamlMarkerCtxExtractor::parseMarkerCtxExtractorSpecific(const YAML::Node &node)
    {
      specificSet       = true;
      YAML::Node others = node["others"][0];
      width             = others["width"].as<int>();
      height            = others["height"].as<int>();
    }


    void YamlMarkerCtxExtractor::printMarkerCtxExtractor()
    {
      printBase();
      printMarkerCtxExtractorSpecific();
    }


    void YamlMarkerCtxExtractor::printMarkerCtxExtractorSpecific()
    {
      std::cout << "Others --------" << std::endl;
      std::cout << "\tFrame Resolution: " << width << "x" << height << std::endl;
    }


    void* YamlMarkerCtxExtractor::make()
    {
      if(baseSet && specificSet)
      {
        kernels::MarkerCtxExtractor *temp = new kernels::MarkerCtxExtractor(id, width, height);
        temp->setFrequency(frequency);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].connectionType == "local")
          {
            temp->activateInPortAsLocal<kernels::CtxExtractorInMarkerType>(inPorts[i].portName);
          }
          else if(inPorts[i].connectionType == "remote")
          {
            temp->activateInPortAsRemote<kernels::CtxExtractorInMarkerType>(inPorts[i].portName,
                                                                            inPorts[i].bindingPortNum);
          }
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          if(outPorts[i].connectionType == "local")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsLocal<kernels::CtxExtractorOutCtxType>(outPorts[i].portName);
            else
              temp->duplicateOutPortAsLocal<kernels::CtxExtractorOutCtxType>(outPorts[i].duplicatedFrom,
                                                                             outPorts[i].portName);
          }
          else if(outPorts[i].connectionType == "remote")
          {
            if(outPorts[i].duplicatedFrom == "")
              temp->activateOutPortAsRemote<kernels::CtxExtractorOutCtxType>(
                  outPorts[i].portName,
                  outPorts[i].connectingAddr,
                  outPorts[i].connectingPortNum);
            else
              temp->duplicateOutPortAsRemote<kernels::CtxExtractorOutCtxType>(
                  outPorts[i].duplicatedFrom,
                  outPorts[i].portName,
                  outPorts[i].connectingAddr,
                  outPorts[i].connectingPortNum);
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

