#ifndef __FLEXR_KEYBOARD__
#define __FLEXR_KEYBOARD__

#include <bits/stdc++.h>
#include <raft>
#include "defs.h"
#include "types/types.h"
#include "kernels/kernel.h"
#include "components/frequency_manager.h"

namespace flexr
{
  namespace kernels
  {
    using KeyboardMsgType=types::Message<char>;

    class Keyboard : public FleXRKernel
    {
    private:
      uint32_t seq;
      components::FrequencyManager freqManager;

    public:
      Keyboard(int frequency=60);
      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace flexr

#endif

