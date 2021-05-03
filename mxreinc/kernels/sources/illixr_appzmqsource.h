#ifndef __MXRE_ILLIXR_APP_ZMQ_SOURCE__
#define __MXRE_ILLIXR_APP_ZMQ_SOURCE__

#include <bits/stdc++.h>
#include <raft>
#include <zmq.h>
#include <opencv2/opencv.hpp>
#include <opencv2/video.hpp>
#include <opencv2/highgui.hpp>

#include "defs.h"
#include "types/clock_types.h"
#include "types/cv/types.h"
#include "utils/cv_utils.h"
#include "kernels/kernel.h"
#include "types/kimera/types.h"
#include <ctime>
#include <chrono>

namespace mxre
{
  namespace kernels
  {

    template<typename OUT_T>
    class IllixrAppZMQSource: public MXREKernel
    {
    protected:
      void *ctx;
      void *sock;

    public:
      IllixrAppZMQSource() {
        addOutputPort<OUT_T>("out_data");
      }


      ~IllixrAppZMQSource() {
        zmq_close(sock);
        zmq_ctx_destroy(ctx);
      }


      void setup(char const* this_machine_ip) {
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_PAIR);
        std::string this_machine_ip_str=std::string(this_machine_ip);
        std::string bindingAddr = std::string("tcp://")+this_machine_ip_str+std::string(":19436");
        zmq_bind(sock, bindingAddr.c_str());

        debug_print("bindingAddr: %s connected\n", bindingAddr.c_str());
      }

      int recvFrame(mxre::types::Frame *data) {
        mxre::types::Frame *frame = (mxre::types::Frame*)data;
        zmq_recv(sock, frame, sizeof(mxre::types::Frame), 0);
        frame->data = new unsigned char[frame->dataSize];

        zmq_recv(sock, frame->data, frame->dataSize, 0);

        return true;
      }

      void print_current_date(void){
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

        profile_print("%u:%u:%u:%u:%u:%u", hours.count(), minutes.count(), seconds.count(), milliseconds.count(), microseconds.count(), nanoseconds.count());
      }

      //sizeof(mxre::types::Frame)=72
      //frame->dataSize=360960
      //sizeof(mxre::types::Frame)=72
      //frame->dataSize=360960
      //sizeof(cam_data->time)=8
      //sizeof(cam_data->imu_count)=4
      //sizeof(cam_data->dataset_time)=8
      void recv_cam_type(OUT_T* cam_data_) {
        print_current_date();

        if (sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRSink is not set." << std::endl;
          return;
        }
        
        mxre::kimera_type::imu_cam_type *cam_data = (mxre::kimera_type::imu_cam_type*) cam_data_;
        //TODO: check Frame creation

        if (cam_data->img0!=NULL){
          cam_data->img0->release();
          cam_data->img1->release();
        }
        else{
          cam_data->img0=new types::Frame();
          cam_data->img1=new types::Frame();
        }

        uint8_t* buffer_cam_metadata = new uint8_t[sizeof(mxre::types::Frame)*2+sizeof(cam_data->time)+sizeof(cam_data->imu_count)+sizeof(cam_data->dataset_time)];
        zmq_recv(sock, buffer_cam_metadata, sizeof(mxre::types::Frame)*2+sizeof(cam_data->time)+sizeof(cam_data->imu_count)+sizeof(cam_data->dataset_time), 0);
        memcpy(cam_data->img0,buffer_cam_metadata,sizeof(mxre::types::Frame));
        memcpy(cam_data->img1,&buffer_cam_metadata[sizeof(mxre::types::Frame)],sizeof(mxre::types::Frame));
        // printf("%lu %lu\n",((mxre::types::Frame*)buffer_cam_metadata)->dataSize,((mxre::types::Frame*)&buffer_cam_metadata[sizeof(mxre::types::Frame)])->dataSize);
        // printf("%lu %lu\n",cam_data->img0->dataSize,cam_data->img1->dataSize);

        memcpy(&(cam_data->time),&buffer_cam_metadata[sizeof(mxre::types::Frame)*2],sizeof(cam_data->time));
        memcpy(&(cam_data->imu_count),&buffer_cam_metadata[sizeof(mxre::types::Frame)*2+sizeof(cam_data->time)],sizeof(cam_data->imu_count));
        memcpy(&(cam_data->dataset_time),&buffer_cam_metadata[sizeof(mxre::types::Frame)*2+sizeof(cam_data->time)+sizeof(cam_data->imu_count)],sizeof(cam_data->dataset_time));
        
        uint8_t* buffer_cam_imu_variable_data = new uint8_t[cam_data->img0->dataSize+cam_data->img1->dataSize+cam_data->imu_count*sizeof(kimera_type::imu_type)];
        zmq_recv(sock, buffer_cam_imu_variable_data, cam_data->img0->dataSize+cam_data->img1->dataSize+cam_data->imu_count*sizeof(kimera_type::imu_type), 0);
        cam_data->img0->data = new unsigned char[cam_data->img0->dataSize];
        cam_data->img1->data = new unsigned char[cam_data->img1->dataSize];
        memcpy(cam_data->img0->data,(void*)buffer_cam_imu_variable_data,cam_data->img0->dataSize);
        memcpy(cam_data->img1->data,(void*)(&buffer_cam_imu_variable_data[cam_data->img0->dataSize]),cam_data->img1->dataSize);
        
        cam_data->imu_readings = std::shared_ptr<kimera_type::imu_type[]>(new kimera_type::imu_type[cam_data->imu_count]);
        memcpy(cam_data->imu_readings.get(),(void*)(&buffer_cam_imu_variable_data[cam_data->img0->dataSize+cam_data->img1->dataSize]),cam_data->imu_count*sizeof(kimera_type::imu_type)),
        // printf("dataset_time imu_count received %lu %u %llu %llu %llu\n",cam_data->dataset_time,cam_data->imu_count,cam_data->imu_readings.get()->dataset_time,cam_data->imu_readings.get()[1].dataset_time,cam_data->imu_readings[2].dataset_time);
        // printf("%lu %lu %llu %llu %llu\n",cam_data->img0->dataSize,cam_data->img1->dataSize,((mxre::kimera_type::imu_type*) &buffer_cam_imu_variable_data[cam_data->img0->dataSize+cam_data->img1->dataSize])->dataset_time,((mxre::kimera_type::imu_type*) &buffer_cam_imu_variable_data[cam_data->img0->dataSize+cam_data->img1->dataSize])[1].dataset_time,((mxre::kimera_type::imu_type*) &buffer_cam_imu_variable_data[cam_data->img0->dataSize+cam_data->img1->dataSize])[2].dataset_time);
        delete[] buffer_cam_metadata;
        delete[] buffer_cam_imu_variable_data;
        //usleep(50000);
        print_current_date();

        return;
      }


      virtual raft::kstatus run() {
#ifdef __PROFILE__
        mxre::types::TimeVal start = getNow();
#endif

        if(sock == NULL || ctx == NULL) {
          debug_print("IllixrAppZMQSource is not set");
          return raft::stop;
        }
        auto &outData(output["out_data"].template allocate<mxre::kimera_type::imu_cam_type>());
        int recvFlag = false;
        recv_cam_type(&outData);

#ifdef __PROFILE__
        mxre::types::TimeVal end = getNow();
        // profile_print("Exe Time ILLIXR Appsource: %lfms", getExeTime(end, start));
#endif
        output["out_data"].send();
        return raft::proceed;
      }
    };

  }   // namespace kernels
} // namespace mxre

#endif

