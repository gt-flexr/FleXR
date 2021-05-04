#ifndef __MXRE_KEYBOARD__
#define __MXRE_KEYBOARD__

#include <bits/stdc++.h>
#include <raft>
#include "defs.h"
#include "types/types.h"
#include "kernels/kernel.h"

namespace mxre
{
  namespace kernels
  {

    class Keyboard : public MXREKernel
    {
    private:
      uint32_t seq;

    public:
      Keyboard();
      ~Keyboard();
      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

