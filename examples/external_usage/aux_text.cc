#include <bits/stdc++.h>
#include <flexr>

using namespace std;

class Temp : public flexr::kernels::FleXRKernel
{
  public:
    Temp(std::string id): flexr::kernels::FleXRKernel(id)
    {
      setName("Temp");
      portManager.registerInPortTag("in_data", flexr::components::PortDependency::BLOCKING);
      portManager.registerOutPortTag("out_data", flexr::utils::sendLocalPointerMsgCopy, flexr::utils::serializeEncodedFrame);
    }

    raft::kstatus run()
    {
      flexr::types::Message<uint8_t*> *inData = portManager.getInput<flexr::types::Message<uint8_t*>>("in_data");
      flexr::types::Message<uint8_t*> *outData = portManager.getOutputPlaceholder<flexr::types::Message<uint8_t*>>("out_data");

      outData->setHeader(inData->tag, inData->seq, inData->ts, inData->dataSize);
      outData->data = inData->data;

      portManager.sendOutput("out_data", outData);

      portManager.freeInput("in_data", inData);

      return raft::proceed;
    }
};


class YamlTemp: public flexr::yaml::YamlFleXRKernel
{
  public:
    YamlTemp()
    {
      specificSet = true;
    }

    void parseTestKernel(const YAML::Node &node)
    {
      parseBase(node);
    }

    void* make()
    {
      if(baseSet && specificSet)
      {
        Temp *temp = new Temp(id);
        if(loggerId != "" && loggerFileName != "") temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_data")
          {
            if(inPorts[i].connectionType == "local")
              temp->activateInPortAsLocal<flexr::types::Message<uint8_t*>>(inPorts[i].portName);
            else
              temp->activateInPortAsRemote<flexr::types::Message<uint8_t*>>(inPorts[i].portName,
                                                                            inPorts[i].protocol,
                                                                            inPorts[i].bindingPortNum);
          }
          else debug_print("invalid input port_name %s for TestKernel", inPorts[i].portName.c_str());
        }

        for(int i = 0; i < outPorts.size(); i++)
        {
          // Kernel specified ports
          if(outPorts[i].portName == "out_data")
          {
            if(outPorts[i].connectionType == "local")
              temp->activateOutPortAsLocal<flexr::types::Message<uint8_t*>>(outPorts[i].portName);
            else if(outPorts[i].connectionType == "remote")
              temp->activateOutPortAsRemote<flexr::types::Message<uint8_t*>>(outPorts[i].portName,
                                                                             outPorts[i].protocol,
                                                                             outPorts[i].connectingAddr,
                                                                             outPorts[i].connectingPortNum);
          }
          else
          {
            // Duplicated ports (non-specified)
            if(outPorts[i].duplicatedFrom == "out_frame")
            {
              if(outPorts[i].connectionType == "local")
                temp->duplicateOutPortAsLocal<flexr::types::Message<uint8_t*>>(outPorts[i].duplicatedFrom,
                                                                               outPorts[i].portName);
              else if(outPorts[i].connectionType == "remote")
                temp->duplicateOutPortAsRemote<flexr::types::Message<uint8_t*>>(outPorts[i].duplicatedFrom,
                                                                                outPorts[i].portName,
                                                                                outPorts[i].protocol,
                                                                                outPorts[i].connectingAddr,
                                                                                outPorts[i].connectingPortNum);
            }
            else debug_print("invalid output port_name %s for FrameDecoder", outPorts[i].portName.c_str());
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
};


bool customKernelInit(YAML::Node recipe, flexr::kernels::FleXRKernel *&outKernel)
{
  if(recipe["kernel"].as<std::string>() == "Temp")
  {
    YamlTemp temp;
    temp.parseTestKernel(recipe);
    temp.printBase();
    outKernel = (Temp*)temp.make();
    return true;
  }

  return false;
}


bool customKernelDestroy(flexr::kernels::FleXRKernel* kernel)
{
  if(kernel->getName() == "Temp")
  {
    delete (Temp*) kernel;
    return true;
  }

  return false;
}


int main(int argc, char **argv)
{
  cxxopts::Options options("FleXR Runner", "FleXR Runner");
  options.add_options()
    ("y, yaml", "YAML file", cxxopts::value<std::string>())
    ("h, help", "Print usage")
  ;

  auto parsedArgs = options.parse(argc, argv);
  if(parsedArgs.count("help"))
  {
    std::cout << options.help() << std::endl;
    exit(0);
  }

  std::string yamlRecipe;
  if(parsedArgs.count("yaml"))
  {
    yamlRecipe = parsedArgs["yaml"].as<std::string>();
    std::cout << "YAML Recipe: " << yamlRecipe << std::endl;
  }
  else
  {
    std::cout << "Invalid YAML recipe" << std::endl;
    exit(0);
  }

  // Parse YAML recipe
  flexr::yaml::YamlFleXRParser yamlParser(yamlRecipe);
  yamlParser.RegisterCustomKernelFuncs(customKernelInit, customKernelDestroy);
  yamlParser.initKernels();
  yamlParser.parseConnections();

  // Instantiate DAG components from the parsed info
  std::map<std::string, flexr::kernels::FleXRKernel*> kernelMap;
  std::vector<flexr::kernels::FleXRKernel*> singleKernels;
  std::vector<flexr::yaml::YamlLocalConnection> connections;
  kernelMap     = yamlParser.getKernelMap();
  singleKernels = yamlParser.getSingleKernels();
  connections   = yamlParser.getConnections();

  raft::map pipeline;
  for(int i = 0; i < connections.size(); i++)
  {
    pipeline.link(kernelMap[connections[i].sendKernel], connections[i].sendPortName,
                  kernelMap[connections[i].recvKernel], connections[i].recvPortName, connections[i].queueSize);
  }

  std::vector<std::thread> singleKernelThreads;
  for(int i = 0; i < singleKernels.size(); i++)
  {
    std::thread singleKernelThread(flexr::kernels::runSingleKernel, singleKernels[i]);
    singleKernelThreads.push_back(std::move(singleKernelThread));
  }

  pipeline.exe();

  for(int i = 0; i < singleKernelThreads.size(); i++) singleKernelThreads[i].join();

  return 0;
}

