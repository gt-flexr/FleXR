#ifndef __MXRE_TYPES__
#define __MXRE_TYPES__

#include <bits/stdc++.h>

// TODO: integrate all the types into this file
namespace mxre {
  namespace types {
    typedef struct RTPTrackingInfo {
      int32_t invoice; // for sanity check
      uint32_t index;
      double timestamp;
    } RTPTrackingInfo;
  }
}
#endif

