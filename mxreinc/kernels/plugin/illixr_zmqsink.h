#ifndef __MXRE_ILLIXR_ZMQ_SINK__
#define __MXRE_ILLIXR_ZMQ_SINK__

#include <bits/stdc++.h>
#include <zmq.h>
#include "defs.h"
#include "types/frame.h"
#include "types/kimera/types.h"
#include <ctime>
#include <chrono>

namespace mxre {
  namespace kernels {

    template<typename OUT_T>
    class ILLIXRZMQSink {
    private:
      void *ctx;
      void *sock;
      int dtype;

    public:
      ILLIXRZMQSink() {
        ctx = NULL;
        sock = NULL;
      }


      ~ILLIXRZMQSink() {
        if(sock) zmq_close(sock);
        if(ctx) zmq_ctx_destroy(ctx);
      }


      void setup(const char* this_machine_ip) {
        this->dtype = dtype;
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_PAIR);
        std::string bindingAddr = std::string("tcp://")+std::string(this_machine_ip)+std::string(":19435");
        zmq_bind(sock, bindingAddr.c_str());
        debug_print("bindingAddr: %s connected\n", bindingAddr.c_str());
      }

      void print_current_date(std::string label){
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();

        typedef std::chrono::duration<int, std::ratio_multiply<std::chrono::hours::period, std::ratio<8>
        >::type> Days; /* UTC: +8:00 */

        Days days = std::chrono::duration_cast<Days>(duration);
            duration -= days;
        auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
            duration -= hours;
        auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
            duration -= minutes;
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
            duration -= seconds;
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            duration -= milliseconds;
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
            duration -= microseconds;
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

        profile_print("%s %u:%u:%u:%u:%u:%u", label.c_str(), hours.count(), minutes.count(), seconds.count(), milliseconds.count(), microseconds.count(), nanoseconds.count());
      }


      void recv_kimera_output(OUT_T* kimera_output_) {
        mxre::types::TimeVal start = getNow();
        
        if (sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRZMQSink is not set." << std::endl;
          return;
        }
        print_current_date("Before recv_kimera_output");
        mxre::kimera_type::kimera_output *kimera_output = (mxre::kimera_type::kimera_output*) kimera_output_;
        zmq_recv(sock, kimera_output, sizeof(mxre::kimera_type::kimera_output), 0);
        print_current_date("After recv_kimera_output");
        mxre::types::TimeVal end = getNow();
        debug_print("Exe Time ILLIXR Appsource: %lfms", getExeTime(end, start));
        
        return;
      }

    };

  }
}

#endif
