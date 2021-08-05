#ifndef __FLEXR_COMPONENTS_LOGGER__
#define __FLEXR_COMPONENTS_LOGGER__

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "defs.h"

namespace flexr
{
  namespace components
  {

    class Logger
    {
      private:
        std::shared_ptr<spdlog::logger> instance;
        int pid;
      public:
        Logger();
        Logger(std::string loggerID, std::string logFileName);
        void set(std::string loggerID, std::string logFileName);
        bool isSet();
        std::shared_ptr<spdlog::logger> getInstance();
        ~Logger();
    };

  }
}
#endif

