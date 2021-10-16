#include <kernels/source/keyboard.h>
#include <utils/getch.h>
#include <utils/local_copy_functions.h>
#include <types/types.h>

namespace flexr
{
  namespace kernels
  {

    Keyboard::Keyboard(std::string id, int frequency): FleXRKernel(id)
    {
      setName("Keyboard");
      seq = 0;
      portManager.registerOutPortTag("out_key",
                                     utils::sendLocalBasicCopy<KeyboardMsgType>);
      freqManager.setFrequency(frequency);
    }


    raft::kstatus Keyboard::run()
    {
      KeyboardMsgType *outKey = portManager.getOutputPlaceholder<KeyboardMsgType>("out_key");

      outKey->data = flexr::utils::getch();
      outKey->setHeader("keystroke", seq++, getTsNow(), sizeof(char));
      outKey->printHeader();

      portManager.sendOutput<KeyboardMsgType>("out_key", outKey);

      debug_print("stroke(%lf): %c", getTsNow(), outKey->data);
      if(logger.isSet()) logger.getInstance()->info("{}th keystroke {} occurs\t {}", seq-1, outKey->data, outKey->ts);

      freqManager.adjust();
      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

