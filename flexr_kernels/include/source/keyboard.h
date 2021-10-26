#pragma once

#ifdef __FLEXR_KERNEL_KEYBOARD__

#include <bits/stdc++.h>
#include <unistd.h>
#include <termios.h>

#include "flexr_core/include/core.h"

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

      char getch();

    public:
      /**
       * @brief Initialize keyboard kernel
       * @param frequency
       *  Target frequency to capture key strokes
       * @see flexr::components::FrequencyManager
       */
      Keyboard(std::string id, int frequency=60);


      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace flexr

#endif

