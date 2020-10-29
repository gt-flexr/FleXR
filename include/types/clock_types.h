#ifndef __MXRE_CLOCK_TYPE__
#define __MXRE_CLOCK_TYPE__

#include <chrono>

namespace mxre {
  namespace types {

    typedef std::chrono::high_resolution_clock::time_point TimeVal;
    typedef struct FrameStamp {
      TimeVal st;
      unsigned int index;
    } FrameStamp;

  } // namespace types
} // namespace mxre
#endif

