#include <kernels/source/keyboard.h>
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

    char Keyboard::getch()
    {
      char buf = 0;
      struct termios old = {0};
      fflush(stdout);
      if(tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
      old.c_lflag &= ~ICANON;
      old.c_lflag &= ~ECHO;
      old.c_cc[VMIN] = 1;
      old.c_cc[VTIME] = 0;
      if(tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
      if(read(0, &buf, 1) < 0)
        perror("read()");
      old.c_lflag |= ICANON;
      old.c_lflag |= ECHO;
      if(tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
      return buf;
    }


    raft::kstatus Keyboard::run()
    {
      KeyboardMsgType *outKey = portManager.getOutputPlaceholder<KeyboardMsgType>("out_key");

      outKey->data = getch();
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

