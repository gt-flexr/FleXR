#include <bits/stdc++.h>
#include <flexr>

using namespace std;

class TestKernel : public flexr::kernels::FleXRKernel
{
  public:
    TestKernel(std::string id): flexr::kernels::FleXRKernel(id)
    {
      setName("TestKernel");
    }

    raft::kstatus run()
    {
      return raft::proceed;
    }
};


class YamlTestKernel: public flexr::yaml::YamlFleXRKernel
{
  public:
    YamlTestKernel()
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
        TestKernel *temp = new TestKernel(id);
        temp->setLogger(loggerId, loggerFileName);

        for(int i = 0; i < inPorts.size(); i++)
        {
          if(inPorts[i].portName == "in_frame")
          {
            if(inPorts[i].connectionType == "local")
              temp->activateInPortAsLocal<int>(inPorts[i].portName);
            else
              temp->activateInPortAsRemote<int>(inPorts[i].portName,
                  inPorts[i].protocol,
                  inPorts[i].bindingPortNum);
          }
          else debug_print("invalid input port_name %s for TestKernel", inPorts[i].portName.c_str());
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
  if(recipe["kernel"].as<std::string>() == "TestKernel")
  {
    YamlTestKernel yamlTestKernel;
    yamlTestKernel.parseTestKernel(recipe);
    yamlTestKernel.printBase();
    outKernel = (TestKernel*)yamlTestKernel.make();
    return true;
  }

  return false;
}


bool customKernelDestroy(flexr::kernels::FleXRKernel* kernel)
{
  if(kernel->getName() == "TestKernel")
  {
    delete (TestKernel*) kernel;
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

