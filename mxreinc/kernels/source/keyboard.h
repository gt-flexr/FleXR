#ifndef __MXRE_KEYBOARD__
#define __MXRE_KEYBOARD__

#include <bits/stdc++.h>
#include <raft>
#include "defs.h"
#include "types/types.h"
#include "kernels/kernel.h"
#include "components/frequency_manager.h"

namespace mxre
{
  namespace kernels
  {
    using KeyboardMsgType=types::Message<char>;

    class Keyboard : public MXREKernel
    {
    private:
      uint32_t seq;
      components::FrequencyManager freqManager;

    public:
      Keyboard(int frequency=20);
      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace mxre

#endif

