#pragma once

#ifdef __FLEXR_KERNEL_KEYBOARD__

#include <bits/stdc++.h>
#include <unistd.h>

#include <flexr_core/include/core.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

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
      Display *d;
      Window win;
      XEvent event;
      Atom closeMessage;

      bool was_it_auto_repeat(Display * d, XEvent * event, int current_type, int next_type);

    public:
      /**
       * @brief Initialize keyboard kernel
       * @param frequency
       *  Target frequency to capture key strokes
       * @see flexr::components::FrequencyManager
       */
      Keyboard(std::string id, int frequency=60);
      ~Keyboard();


      virtual raft::kstatus run();
    };

  }   // namespace kernels
} // namespace flexr

#endif

