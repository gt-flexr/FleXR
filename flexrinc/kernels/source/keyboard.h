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


    /**
     * @brief Kernel to get key strokes
     *
     * Port Tag       | Type
     * ---------------| ----------------------------
     * out_key        | @ref flexr::types::Message<char>
     */
    class Keyboard : public FleXRKernel
    {
    private:
      uint32_t seq;

    public:
      /**
       * @brief Initialize keyboard kernel
       * @param frequency
       *  Target frequency to capture key strokes
       * @see flexr::components::FrequencyManager
       */
      Keyboard(int frequency=60);


      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace flexr

#endif

