#include <kernels/source/keyboard.h>
#include <utils/getch.h>

namespace mxre
{
  namespace kernels
  {
    Keyboard::Keyboard(std::string tag): MXREKernel(tag)
    {
      seq = 0;
      output.addPort<types::Message<char>>("out_key");
    }


    Keyboard::~Keyboard() {}


    raft::kstatus Keyboard::run() {
      types::Message<char> &key = output["out_key"].template allocate<types::Message<char>>();

      key.data = mxre::utils::getch();
      key.tag  = tag;
      key.seq  = seq++;
      key.ts   = getTimeStampNow();

      sendPrimitiveCopy<types::Message<char>>("out_key", key);

      return raft::proceed;
    }

  } // namespace kernels
} // namespace mxre

