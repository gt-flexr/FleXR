#ifndef __MXRE_KEYBOARD__
#define __MXRE_KEYBOARD__

#include <bits/stdc++.h>
#include <raft>
#include "defs.h"

namespace mxre
{
  namespace kernels
  {

    class Keyboard : public raft::kernel
    {
    public:
      Keyboard();
      ~Keyboard();
      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

