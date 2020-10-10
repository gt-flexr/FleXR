#include "device/keyboard.h"
#include "utils/getch.h"

namespace mxre
{
  namespace pipeline
  {
    namespace device
    {
      Keyboard::Keyboard() {
        output.addPort<char>("out_keystroke");
      }


      Keyboard::~Keyboard() {}


      raft::kstatus Keyboard::run() {
        auto keystroke(output["out_keystroke"].template allocate_s<char>());

        (*keystroke) = utils::getch();

        return raft::proceed;
      }

    } // namespace device
  }   // namespace pipeline
} // namespace mxre

