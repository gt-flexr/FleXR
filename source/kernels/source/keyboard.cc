#include <kernels/source/keyboard.h>
#include <utils/getch.h>
#include <utils/msg_sending_functions.h>
#include <types/types.h>

namespace flexr
{
  namespace kernels
  {

    Keyboard::Keyboard(int frequency): FleXRKernel()
    {
      setName("Keyboard");
      seq = 0;
      portManager.registerOutPortTag("out_key",
                                     utils::sendLocalBasicCopy<KeyboardMsgType>,
                                     utils::sendRemotePrimitive<KeyboardMsgType>,
                                     types::freePrimitiveMsg<KeyboardMsgType>);
      freqManager.setFrequency(frequency);
    }


    raft::kstatus Keyboard::run() {
      KeyboardMsgType *outKey = portManager.getOutputPlaceholder<KeyboardMsgType>("out_key");

      strcpy(outKey->tag, "keystroke");
      outKey->seq  = seq++;
      outKey->ts   = getTsNow();
      outKey->data = flexr::utils::getch();

      debug_print("stroke(%lf): %c", getTsNow(), outKey->data);
      if(logger.isSet()) logger.getInstance()->info("{}th keystroke {} occurs\t {}", seq-1, outKey->data, outKey->ts);

      portManager.sendOutput<KeyboardMsgType>("out_key", outKey);
      freqManager.adjust();

      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

