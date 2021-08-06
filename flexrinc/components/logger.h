#ifndef __FLEXR_COMPONENTS_LOGGER__
#define __FLEXR_COMPONENTS_LOGGER__

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "defs.h"

namespace flexr
{
  namespace components
  {

    /**
     * @brief Component to log events of a kernel
     */
    class Logger
    {
      private:
        std::shared_ptr<spdlog::logger> instance;
        int pid;
      public:
        Logger();
        ~Logger();


        /**
         * @brief Initialize logger with ID and logger file name
         * @param loggerID
         *  Logger identifier
         * @param logFileName
         *  File name of the saved log
         */
        Logger(std::string loggerID, std::string logFileName);


        /**
         * @brief Set logger
         * @param loggerID
         *  Logger identifier
         * @param logFileName
         *  File name of the saved log
         */
        void set(std::string loggerID, std::string logFileName);


        /**
         * @brief Check the loger is set
         * @return Boolean whether a logger is set or not
         */
        bool isSet();


        // Function
        /**
         * @brief Return the logger instance to log
         * @return Logger instance if it is set
         */
        std::shared_ptr<spdlog::logger> getInstance();
    };

  }
}
#endif

