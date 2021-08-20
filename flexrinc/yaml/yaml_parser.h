#ifndef __FLEXR_YAML_PARSER__
#define __FLEXR_YAML_PARSER__

#include <bits/stdc++.h>
#include <yaml-cpp/yaml.h>

#include "defs.h"

#include "flexrinc/kernels/kernel.h"

#include "yaml_flexr_kernel.h"
#include "yaml_ports.h"
#include "yaml_local_connection.h"

#include "flexrinc/yaml/kernels/source/yaml_bag_camera.h"
#include "flexrinc/yaml/kernels/source/yaml_cv_camera.h"
#include "flexrinc/yaml/kernels/source/yaml_keyboard.h"
#include "flexrinc/yaml/kernels/source/yaml_rtp_frame_receiver.h"

#include "flexrinc/yaml/kernels/sink/yaml_rtp_frame_sender.h"
#include "flexrinc/yaml/kernels/sink/yaml_cv_display.h"
#include "flexrinc/yaml/kernels/sink/yaml_non_display.h"

#include "flexrinc/yaml/kernels/intermediate/yaml_aruco_cam_locator.h"
#include "flexrinc/yaml/kernels/intermediate/yaml_aruco_detector.h"
#include "flexrinc/yaml/kernels/intermediate/yaml_cuda_orb_detector.h"
#include "flexrinc/yaml/kernels/intermediate/yaml_orb_detector.h"
#include "flexrinc/yaml/kernels/intermediate/yaml_frame_converter.h"
#include "flexrinc/yaml/kernels/intermediate/yaml_marker_ctx_extractor.h"
#include "flexrinc/yaml/kernels/intermediate/yaml_object_renderer.h"

namespace flexr
{
  namespace yaml
  {

    /**
     * @brief Yaml FleXR Parser
     */
    class YamlFleXRParser
    {
      protected:
        YAML::Node doc;
        std::map<std::string, kernels::FleXRKernel*> parsedKernelMap;
        std::vector<YamlLocalConnection> parsedConnections;


      public:
        /**
         * @brief Init parser with YAML file
         * @param node
         *  YAML node to parse
         */
        YamlFleXRParser(std::string yamlFile)
        {
          loadYamlFile(yamlFile);
        }


        ~YamlFleXRParser()
        {
          removeParsedInfo();
        }


        /**
         * @brief Load YAML file
         * @param yamlFile
         *  YAML file to load
         */
        void loadYamlFile(std::string yamlFile)
        {
          doc = YAML::LoadFile(yamlFile);
        }


        /**
         * @brief Get instantiated kernel map
         * @return Kernel map
         */
        std::map<std::string, flexr::kernels::FleXRKernel*> getKernelMap()
        {
          return parsedKernelMap;
        }


        /**
         * @brief Get parsed connections
         * @param Parsed connections
         */
        std::vector<YamlLocalConnection> getConnections()
        {
          return parsedConnections;
        }


        /**
         * @brief Remove parsed info and instantiated kernels
         */
        void removeParsedInfo()
        {
          removeParsedConnections();
          removeKernels();
          removeKernelMap();
        }


        /**
         * @brief Remove parsed connections
         */
        void removeParsedConnections()
        {
          parsedConnections.clear();
        }


        /**
         * @brief Remove kernel map
         */
        void removeKernelMap()
        {
          parsedKernelMap.clear();
        }


        /**
         * @brief Remove instantiated kernels
         */
        void removeKernels()
        {
          std::map<std::string, flexr::kernels::FleXRKernel*>::iterator iter;
          for(iter = parsedKernelMap.begin(); iter != parsedKernelMap.end(); iter++)
          {
            // Source kernels
            if(iter->second->getName() == "BagCamera") delete (flexr::kernels::BagCamera*)iter->second;
            if(iter->second->getName() == "CVCamera") delete (flexr::kernels::CVCamera*)iter->second;
            if(iter->second->getName() == "Keyboard") delete (flexr::kernels::Keyboard*)iter->second;
            if(iter->second->getName() == "RTPFrameReceiver") delete (flexr::kernels::RTPFrameReceiver*)iter->second;

            // Sink kernels
            if(iter->second->getName() == "CVDisplay") delete (flexr::kernels::CVDisplay*)iter->second;
            if(iter->second->getName() == "NonDisplay") delete (flexr::kernels::NonDisplay*)iter->second;
            if(iter->second->getName() == "RTPFrameSender") delete (flexr::kernels::RTPFrameSender*)iter->second;

            // Intermediate kernels
            if(iter->second->getName() == "FrameConverter") delete (flexr::kernels::FrameConverter*)iter->second;
            if(iter->second->getName() == "MarkerCtxExtractor") delete (flexr::kernels::MarkerCtxExtractor*)iter->second;
            if(iter->second->getName() == "CudaORBDetector") delete (flexr::kernels::CudaORBDetector*)iter->second;
            if(iter->second->getName() == "ORBDetector") delete (flexr::kernels::ORBDetector*)iter->second;
            if(iter->second->getName() == "ObjectRenderer") delete (flexr::kernels::ObjectRenderer*)iter->second;
            if(iter->second->getName() == "ArUcoDetector") delete (flexr::kernels::ArUcoDetector*)iter->second;
            if(iter->second->getName() == "ArUcoCamLocator") delete (flexr::kernels::ArUcoCamLocator*)iter->second;
          }
        }


        /**
         * @brief Initialize and instantiate kernels while parsing YAML recipe
         * @see flexr::yaml::YamlBagCamera, flexr::yaml::YamlCvCamera, flexr::yaml::YamlKeyboard,
         * flexr::yaml::YamlRtpFrameReceiver, flexr::yaml::YamlCvDisplay, flexr::yaml::YamlNonDisplay,
         * flexr::yaml::YamlRtpFrameSender, flexr::yaml::YamlFrameConverter, flexr::yaml::YamlMarkerCtxExtractor,
         * flexr::yaml::YamlCudaOrbDetector, flexr::yaml::YamlOrbDetector, flexr::yaml::YamlObjectRenderer,
         * flexr::yaml::YamlArUcoDetector, flexr::yaml::YamlArUcoCamLocator
         */
        void initKernels()
        {
          for(int i = 0; i < doc.size(); i++)
          {
            if(doc[i]["kernel"].IsDefined())
            {
              kernels::FleXRKernel* temp = nullptr;

              // Source kernels
              if(doc[i]["kernel"].as<std::string>() == "BagCamera")
              {
                YamlBagCamera yamlBagCamera;
                yamlBagCamera.parseBagCamera(doc[i]);
                yamlBagCamera.printBagCamera();
                temp = (kernels::FleXRKernel*)yamlBagCamera.make();
              }
              if(doc[i]["kernel"].as<std::string>() == "CVCamera")
              {
                YamlCvCamera yamlCvCamera;
                yamlCvCamera.parseCvCamera(doc[i]);
                yamlCvCamera.printCvCamera();
                temp = (kernels::CVCamera*)yamlCvCamera.make();
              }
              if(doc[i]["kernel"].as<std::string>() == "Keyboard")
              {
                YamlKeyboard yamlKeyboard;
                yamlKeyboard.parseKeyboard(doc[i]);
                yamlKeyboard.printKeyboard();
                temp = (kernels::Keyboard*)yamlKeyboard.make();
              }
              if(doc[i]["kernel"].as<std::string>() == "RTPFrameReceiver")
              {
                YamlRtpFrameReceiver yamlRtpFrameReceiver;
                yamlRtpFrameReceiver.parseRtpFrameReceiver(doc[i]);
                yamlRtpFrameReceiver.printRtpFrameReceiver();
                temp = (kernels::RTPFrameReceiver*)yamlRtpFrameReceiver.make();
              }

              // Sink kernels
              if(doc[i]["kernel"].as<std::string>() == "CVDisplay")
              {
                YamlCvDisplay yamlCvDisplay;
                yamlCvDisplay.parseCvDisplay(doc[i]);
                yamlCvDisplay.printCvDisplay();
                temp = (kernels::CVDisplay*)yamlCvDisplay.make();
              }
              if(doc[i]["kernel"].as<std::string>() == "NonDisplay")
              {
                YamlNonDisplay yamlNonDisplay;
                yamlNonDisplay.parseNonDisplay(doc[i]);
                yamlNonDisplay.printNonDisplay();
                temp = (kernels::CVDisplay*)yamlNonDisplay.make();
              }
              if(doc[i]["kernel"].as<std::string>() == "RTPFrameSender")
              {
                YamlRtpFrameSender yamlRtpFrameSender;
                yamlRtpFrameSender.parseRtpFrameSender(doc[i]);
                yamlRtpFrameSender.printRtpFrameSender();
                temp = (kernels::RTPFrameSender*)yamlRtpFrameSender.make();
              }

              // Intermediate kernels
              if(doc[i]["kernel"].as<std::string>() == "FrameConverter")
              {
                YamlFrameConverter yamlFrameConverter;
                yamlFrameConverter.parseFrameConverter(doc[i]);
                yamlFrameConverter.printFrameConverter();
                temp = (kernels::FrameConverter*)yamlFrameConverter.make();
              }
              if(doc[i]["kernel"].as<std::string>() == "MarkerCtxExtractor")
              {
                YamlMarkerCtxExtractor yamlMarkerCtxExtractor;
                yamlMarkerCtxExtractor.parseMarkerCtxExtractor(doc[i]);
                yamlMarkerCtxExtractor.printMarkerCtxExtractor();
                temp = (kernels::MarkerCtxExtractor*)yamlMarkerCtxExtractor.make();
              }
              if(doc[i]["kernel"].as<std::string>() == "CudaORBDetector")
              {
                YamlCudaOrbDetector yamlCudaOrbDetector;
                yamlCudaOrbDetector.parseCudaOrbDetector(doc[i]);
                yamlCudaOrbDetector.printCudaOrbDetector();
                temp = (kernels::CudaORBDetector*)yamlCudaOrbDetector.make();
              }
              if(doc[i]["kernel"].as<std::string>() == "ORBDetector")
              {
                YamlOrbDetector yamlOrbDetector;
                yamlOrbDetector.parseOrbDetector(doc[i]);
                yamlOrbDetector.printOrbDetector();
                temp = (kernels::ORBDetector*)yamlOrbDetector.make();
              }
              if(doc[i]["kernel"].as<std::string>() == "ObjectRenderer")
              {
                YamlObjectRenderer yamlObjectRenderer;
                yamlObjectRenderer.parseObjectRenderer(doc[i]);
                yamlObjectRenderer.printObjectRenderer();
                temp = (kernels::ObjectRenderer*)yamlObjectRenderer.make();
              }
              if(doc[i]["kernel"].as<std::string>() == "ArUcoDetector")
              {
                YamlArUcoDetector yamlArUcoDetector;
                yamlArUcoDetector.parseArUcoDetector(doc[i]);
                yamlArUcoDetector.printArUcoDetector();
                temp = (kernels::ArUcoDetector*)yamlArUcoDetector.make();
              }
              if(doc[i]["kernel"].as<std::string>() == "ArUcoCamLocator")
              {
                YamlArUcoCamLocator yamlArUcoCamLocator;
                yamlArUcoCamLocator.parseArUcoCamLocator(doc[i]);
                yamlArUcoCamLocator.printArUcoCamLocator();
                temp = (kernels::ArUcoCamLocator*)yamlArUcoCamLocator.make();
              }

              if(temp != nullptr)
              {
                parsedKernelMap[temp->getId()] = temp;
              }
            }
          }
        }


        /**
         * @brief Parse local connections from YAML recipe
         * @see flexr::yaml::YamlLocalConnection
         */
        void parseConnections()
        {
          for(int i = 0; i < doc.size(); i++)
          {
            if(doc[i]["local_connections"].IsDefined())
            {
              YAML::Node connectionNode = doc[i]["local_connections"];
              for(int i = 0; i < connectionNode.size(); i++)
              {
                YamlLocalConnection temp;
                temp.sendKernel   = connectionNode[i]["send_kernel"].as<std::string>();
                temp.sendPortName = connectionNode[i]["send_port_name"].as<std::string>();
                temp.recvKernel   = connectionNode[i]["recv_kernel"].as<std::string>();
                temp.recvPortName = connectionNode[i]["recv_port_name"].as<std::string>();
                temp.queueSize    = connectionNode[i]["queue_size"].as<int>();
                temp.printConnection();
                std::cout << std::endl;
                parsedConnections.push_back(temp);
              }
            }
          }
        }

    };
  }
}

#endif

