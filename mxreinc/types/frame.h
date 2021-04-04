#ifndef __MXRE_FRAME__
#define __MXRE_FRAME__

#include <cv.hpp>
#include <bits/stdc++.h>
#include "defs.h"
extern "C" {
#include <libavcodec/avcodec.h>
}

namespace mxre {
  namespace types {
    typedef struct FrameTrackingInfo {
    uint32_t index;
    double timestamp;
    } FrameTrackingInfo;


    typedef struct AVEncodedFrame {
      FrameTrackingInfo trackingInfo;
      AVPacket* encodedFrame;
    } AVEncodedFrame;


    class Frame {
    public:
      size_t rows, cols;
      size_t totalElem, elemSize, dataSize;
      int type;
      unsigned char *data;

      // index and timestamp are necessary for tracking
      uint32_t index;
      double timestamp;

      Frame() {
        rows = cols = type = totalElem = elemSize = dataSize = 0;
        index = 0;
        timestamp = 0;
        data = NULL;
      }

      Frame(cv::Mat inMat, uint32_t index, double timestamp) {
        this->index = index;
        this->timestamp = timestamp;

        rows = inMat.rows;
        cols = inMat.cols;
        type = inMat.type();
        totalElem = inMat.total();
        elemSize = inMat.elemSize();
        dataSize = totalElem * elemSize;
        data = new unsigned char[dataSize];
        memcpy(data, inMat.data, dataSize);
      }

      Frame(int rows, int cols, int type, uint32_t index, double timestamp) {
        this->index = index;
        this->timestamp = timestamp;

        this->rows = rows;
        this->cols = cols;
        this->type = type;
        cv::Mat temp(rows, cols, type);
        this->totalElem = temp.total();
        this->elemSize = temp.elemSize();
        this->dataSize = temp.total() * temp.elemSize();
        this->data = new unsigned char[totalElem * elemSize];
      }

      Frame(int rows, int cols, int type, void *data, uint32_t index, double timestamp) {
        this->index = index;
        this->timestamp = timestamp;

        this->rows = rows;
        this->cols = cols;
        this->type = type;
        cv::Mat temp(rows, cols, type);
        this->totalElem = temp.total();
        this->elemSize = temp.elemSize();
        this->dataSize = temp.total() * temp.elemSize();
        this->data = (unsigned char*)data;
      }

      void release() {
        rows = cols = type = totalElem = elemSize = dataSize = 0;
        if(data) delete [] data;
        data = NULL;
      }

      cv::Mat useAsCVMat() { return cv::Mat(rows, cols, type, data); }

      Frame clone() {
        Frame cloneFrame(rows, cols, type, index, timestamp);
        memcpy(cloneFrame.data, data, dataSize);
        return cloneFrame;
      }

      void setFrameAttribFromCVMat(cv::Mat mat) {
        rows = mat.rows;
        cols = mat.cols;
        totalElem = mat.total();
        elemSize = mat.elemSize();
        dataSize = mat.total() * mat.elemSize();
        type = mat.type();
      }

      void printInfo() {
        debug_print("idx(%d/%f) width(%ld) height(%ld) elem(%ld/%ld) dataSize(%ld)",
            index, timestamp, cols, rows, elemSize, totalElem, dataSize);
      }
    };

  } // namespace types
} // namespace mxre
#endif

