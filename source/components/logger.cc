#include <components/logger.h>
#include <bits/stdc++.h>
#include <unistd.h>

namespace mxre {
  namespace components {
    Logger::Logger()
    {
      pid = getpid();
      instance = nullptr;
    }

    Logger::Logger(std::string loggerID, std::string logFileName)
    {
      pid = getpid();
      setLogger(loggerID, logFileName);
    }

    void Logger::setLogger(std::string loggerID, std::string logFileName)
    {
      instance = spdlog::basic_logger_st(loggerID, "mxre_logs/" + std::to_string(pid) + "/" + logFileName);
    }

    bool Logger::isSet()
    {
      if (instance != nullptr) return true;
      else return false;
    }

    std::shared_ptr<spdlog::logger> Logger::getInstance()
    {
      if(instance != nullptr)
        return instance;
      else {
        debug_print("Logger instance is not set. return nullptr");
        return nullptr;
      }
    }

    Logger::~Logger()
    {
      instance->flush();
    }
  }
}

