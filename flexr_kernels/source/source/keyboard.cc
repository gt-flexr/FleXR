#ifdef __FLEXR_KERNEL_KEYBOARD__

#include <flexr_core/include/core.h>
#include <flexr_kernels/include/kernels.h>

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

      d = XOpenDisplay(NULL);
      win = XCreateWindow(d, RootWindow(d, 0), 1, 1, 1, 1, 0, 0, InputOutput, NULL, 0, NULL);
      XSelectInput(d, win, KeyPressMask | KeyReleaseMask | ClientMessage);
      XMapWindow(d, win);
      XFlush(d);
      XEvent event;
      Atom closeMessage = XInternAtom(d, "WM_DELETE_WINDOW", True);
      XSetWMProtocols(d, win, &closeMessage, 1);
    }


    Keyboard::~Keyboard()
    {
      XDestroyWindow(d, win);
      XCloseDisplay(d);
    }


    bool Keyboard::was_it_auto_repeat(Display * d, XEvent * event, int current_type, int next_type){
      /*  Holding down a key will cause 'autorepeat' to send fake keyup/keydown events, but we want to ignore these: '*/
      if(event->type == current_type && XEventsQueued(d, QueuedAfterReading)){
        XEvent nev;
        XPeekEvent(d, &nev);
        return (nev.type == next_type && nev.xkey.time == event->xkey.time && nev.xkey.keycode == event->xkey.keycode);
      }
      return false;
    }


    raft::kstatus Keyboard::run()
    {
      KeyboardMsgType *outKey = portManager.getOutputPlaceholder<KeyboardMsgType>("out_key");

      outKey->data = 0;
      outKey->setHeader("keystroke", seq++, getTsNow(), sizeof(char));

      XNextEvent(d, &event);
      switch(event.type) {
        case KeyPress: {
          fprintf(stdout, "key (%c) was pressed.\n", XLookupKeysym(&event.xkey, 0));
          outKey->data = XLookupKeysym(&event.xkey, 0);
          break;
        }case KeyRelease:{
          if(was_it_auto_repeat(d, &event, KeyRelease, KeyPress)){
            XNextEvent(d, &event); /* Consume the extra event so we can ignore it. */
            fprintf(stdout, "key (%c) was still pressed.\n", XLookupKeysym(&event.xkey, 0));
            outKey->data = XLookupKeysym(&event.xkey, 0);
          }else{
            fprintf(stdout, "key (%c) was released.\n", XLookupKeysym(&event.xkey, 0));
            outKey->data = 0;
          }
          break;
        }case ClientMessage:{
          if ((Atom)event.xclient.data.l[0] == closeMessage) {
            exit(1);
          }
          break;
        }
      }

      portManager.sendOutput<KeyboardMsgType>("out_key", outKey);

      if(logger.isSet()) logger.getInstance()->info("{}th keystroke {} occurs\t {}", seq-1, outKey->data, outKey->ts);

      freqManager.adjust();
      return raft::proceed;
    }

  } // namespace kernels
} // namespace flexr

#endif

