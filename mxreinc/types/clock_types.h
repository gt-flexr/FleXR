#ifndef __MXRE_CLOCK_TYPE__
#define __MXRE_CLOCK_TYPE__

#include <chrono>

namespace mxre {
  namespace types {

    typedef std::chrono::high_resolution_clock::time_point TimeVal;

    typedef struct Profile {
      unsigned int index;
      TimeVal st;
    } Profile;

  } // namespace types
} // namespace mxre
#endif

