#pragma once

#include <opencv4/opencv2/opencv.hpp>
#include <bits/stdc++.h>
#include <flexr_core/include/defs.h>


namespace flexr
{
  namespace types
  {
    class Frame
    {
    public:
      size_t rows, cols;
      size_t totalElem, elemSize, dataSize;
      int type;
      unsigned char *data;

      Frame() {
        rows = cols = type = totalElem = elemSize = dataSize = 0;
        data = NULL;
      }

      Frame(cv::Mat inMat) {
        copyFromCvMat(inMat);
      }

      Frame(int rows, int cols, int type) {
        this->rows = rows;
        this->cols = cols;
        this->type = type;
        cv::Mat temp(rows, cols, type);
        this->totalElem = temp.total();
        this->elemSize = temp.elemSize();
        this->dataSize = temp.total() * temp.elemSize();
        this->data = new unsigned char[totalElem * elemSize];
      }

      Frame(int rows, int cols, int type, void *data) {
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
        Frame cloneFrame(rows, cols, type);
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


      void copyFromCvMat(cv::Mat inMat)
      {
        rows = inMat.rows;
        cols = inMat.cols;
        type = inMat.type();
        totalElem = inMat.total();
        elemSize = inMat.elemSize();
        dataSize = totalElem * elemSize;
        data = new unsigned char[dataSize];
        memcpy(data, inMat.data, dataSize);
      }

      void printInfo() {
        debug_print("width(%ld) height(%ld) elem(%ld/%ld) dataSize(%ld)", cols, rows, elemSize, totalElem, dataSize);
      }
    };

  } // namespace types
} // namespace flexr

