#include <kernels/sources/keyboard.h>
#include <utils/getch.h>

namespace mxre
{
  namespace kernels
  {
    Keyboard::Keyboard() {
      output.addPort<char>("out_keystroke");
    }


    Keyboard::~Keyboard() {}


    raft::kstatus Keyboard::run() {
      auto keystroke(output["out_keystroke"].template allocate_s<char>());

      (*keystroke) = mxre::utils::getch();

      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

