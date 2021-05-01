#ifndef __MXRE_COMPONENTS_LOGGER__
#define __MXRE_COMPONENTS_LOGGER__

#include <mxre>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace mxre {
  namespace components {
    class Logger {
    private:
      std::shared_ptr<spdlog::logger> instance;
      int pid;
    public:
      Logger();
      Logger(std::string loggerID, std::string logFileName);
      void setLogger(std::string loggerID, std::string logFileName);
      bool isSet();
      std::shared_ptr<spdlog::logger> getInstance();
      ~Logger();
    };
  }
}
#endif

