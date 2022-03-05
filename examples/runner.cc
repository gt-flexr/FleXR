#include <bits/stdc++.h>
#include <flexr>

using namespace std;

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
  // yamlParser.RegisterCustomKernelFuncs(std::function<bool (std::string, flexr::kernels::FleXRKernel *&)> initFunc,
  //                                      std::function<bool (flexr::kernels::FleXRKernel *)> destroyFunc)
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

