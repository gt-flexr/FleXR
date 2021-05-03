#ifndef __MXRE_ILLIXR_ZMQ_SOURCE__
#define __MXRE_ILLIXR_ZMQ_SOURCE__

#include <bits/stdc++.h>
#include <zmq.h>
#include <opencv/cv.hpp>
#include "defs.h"
#include "types/frame.h"
#undef Success
#include "types/kimera/types.h"
#include <ctime>
#include <chrono>


namespace mxre {
  namespace kernels {

    template<typename IN_T>
    class ILLIXRZMQSource {
    private:
      void *ctx;
      void *sock;
      int dtype;
      char ack[4];

    public:
      ILLIXRZMQSource() {
        ctx = NULL;
        sock = NULL;
      }


      ~ILLIXRZMQSource(){
          if(sock) zmq_close(sock);
          if(ctx) zmq_ctx_destroy(ctx);
      }


      void setup(const char* dest_machine_ip, int dtype=MXRE_DTYPE_IMU_CAM) {
        this->dtype = dtype;
        ctx = zmq_ctx_new();
        sock = zmq_socket(ctx, ZMQ_PAIR);
        std::string connectAddr = std::string("tcp://")+std::string(dest_machine_ip)+std::string(":19436");
        zmq_connect(sock, connectAddr.c_str());
        debug_print("connetingAddr: %s connected\n", connectAddr.c_str());
      }

      // int send_imu_type(OUT_T* data) {
      //   if (sock == NULL || ctx == NULL) {
      //     std::cerr << "ILLIXRZMQSource is not set." << std::endl;
      //     return -1;
      //   }

      //   std::vector<mxre::kimera_type::imu_type> *imu_data = (std::vector<mxre::kimera_type::imu_type> *) data;
      //   debug_print("Sending IMU Values");



      //   zmq_recv(sock, ack, sizeof(ack), 0);
      //   return 1;
      // }

      void print_current_date(string label){
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

        profile_print("%s %u:%u:%u:%u:%u:%u", label, hours.count(), minutes.count(), seconds.count(), milliseconds.count(), microseconds.count(), nanoseconds.count());
      }

      void sendFrame(mxre::types::Frame *data) {
        mxre::types::Frame *frame = (mxre::types::Frame*)data;
        //cv::Mat *mat = (cv::Mat*)data;
        //mxre::types::Frame frame;
        //frame.setFrameAttribFromCVMat(*mat);
        // debug_print("%d %d %d %d", frame->cols, frame->rows, frame->dataSize, frame->totalElem);

        zmq_send(sock, frame, sizeof(mxre::types::Frame), 0);
        zmq_send(sock, frame->data, frame->dataSize, 0);
        frame->release();
      }

      int send_cam_imu_type(IN_T* data) {
        print_current_date("1A");
        if (sock == NULL || ctx == NULL) {
          std::cerr << "ILLIXRZMQSource is not set." << std::endl;
          return -1;
        }
        
        mxre::kimera_type::imu_cam_type *cam_data = (mxre::kimera_type::imu_cam_type*) data;

        uint8_t* buffer_cam_metadata = new uint8_t[sizeof(mxre::types::Frame)*2+sizeof(cam_data->time)+sizeof(cam_data->imu_count)+sizeof(cam_data->dataset_time)];
        memcpy(buffer_cam_metadata,cam_data->img0,sizeof(mxre::types::Frame));
        memcpy(&buffer_cam_metadata[sizeof(mxre::types::Frame)],cam_data->img1,sizeof(mxre::types::Frame));
        memcpy(&buffer_cam_metadata[sizeof(mxre::types::Frame)*2],&(cam_data->time),sizeof(cam_data->time));
        memcpy(&buffer_cam_metadata[sizeof(mxre::types::Frame)*2+sizeof(cam_data->time)],&(cam_data->imu_count),sizeof(cam_data->imu_count));
        memcpy(&buffer_cam_metadata[sizeof(mxre::types::Frame)*2+sizeof(cam_data->time)+sizeof(cam_data->imu_count)],&(cam_data->dataset_time),sizeof(cam_data->dataset_time));
        zmq_send(sock, buffer_cam_metadata, sizeof(mxre::types::Frame)*2+sizeof(cam_data->time)+sizeof(cam_data->imu_count)+sizeof(cam_data->dataset_time), 0);
        
        uint8_t* buffer_cam_imu_variable_data = new uint8_t[cam_data->img0->dataSize+cam_data->img1->dataSize+cam_data->imu_count*sizeof(mxre::kimera_type::imu_type)];
        memcpy((void*)buffer_cam_imu_variable_data,cam_data->img0->data,cam_data->img0->dataSize);
        memcpy((void*)(&buffer_cam_imu_variable_data[cam_data->img0->dataSize]),cam_data->img1->data,cam_data->img1->dataSize);
        
        memcpy((void*)(&buffer_cam_imu_variable_data[cam_data->img0->dataSize+cam_data->img1->dataSize]),cam_data->imu_readings.get(),cam_data->imu_count*sizeof(mxre::kimera_type::imu_type)),
        zmq_send(sock, buffer_cam_imu_variable_data, cam_data->img0->dataSize+cam_data->img1->dataSize+cam_data->imu_count*sizeof(mxre::kimera_type::imu_type), 0);        
        // printf("dataset_time imu_count sent %lu %u %llu %llu %llu\n",cam_data->dataset_time,cam_data->imu_count,cam_data->imu_readings.get()->dataset_time,cam_data->imu_readings.get()[1].dataset_time,cam_data->imu_readings[2].dataset_time);
        // printf("%llu %llu %llu\n",((mxre::kimera_type::imu_type*) &buffer_cam_imu_variable_data[cam_data->img0->dataSize+cam_data->img1->dataSize])->dataset_time,((mxre::kimera_type::imu_type*) &buffer_cam_imu_variable_data[cam_data->img0->dataSize+cam_data->img1->dataSize])[1].dataset_time,((mxre::kimera_type::imu_type*) &buffer_cam_imu_variable_data[cam_data->img0->dataSize+cam_data->img1->dataSize])[2].dataset_time);
        delete[] buffer_cam_metadata;
        delete[] buffer_cam_imu_variable_data;

        print_current_date("1B");

        // sendFrame(cam_data->img0);
        // sendFrame(cam_data->img1);

        // zmq_send(sock, &(cam_data->time), sizeof(cam_data->time), 0);
        // zmq_send(sock, &(cam_data->imu_count), sizeof(cam_data->imu_count), 0);
        // zmq_send(sock, &(cam_data->dataset_time) , sizeof(cam_data->dataset_time), 0);
        // zmq_send(sock, cam_data->imu_readings.get(), cam_data->imu_count * sizeof(mxre::kimera_type::imu_type), 0);
        // zmq_recv(sock, ack, sizeof(ack), 0);

        // debug_print("ILLIXR SENDING DATA FROM MXRE (1), DATASET TIME: %llu", cam_data->dataset_time);
        return 1;
      }
    };

  }
}

#endif

