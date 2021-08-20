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
          if(inPorts[i].portName == "in_detected_markers") {
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
          else debug_print("invalid input port_name %s for MarkerCtxExtractor", inPorts[i].portName.c_str());
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_marker_contexts")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<kernels::CtxExtractorOutCtxType>(outPorts[i].portName);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<kernels::CtxExtractorOutCtxType>(outPorts[i].portName,
                                                                             outPorts[i].connectingAddr,
                                                                             outPorts[i].connectingPortNum);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_frame")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<kernels::CtxExtractorOutCtxType>(outPorts[i].duplicatedFrom,
                                                                             outPorts[i].portName);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<kernels::CtxExtractorOutCtxType>(
                    outPorts[i].duplicatedFrom, outPorts[i].portName,
                    outPorts[i].connectingAddr, outPorts[i].connectingPortNum);
            }
            else debug_print("invalid output port_name %s for MarkerCtxExtractor", outPorts[i].portName.c_str());
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

