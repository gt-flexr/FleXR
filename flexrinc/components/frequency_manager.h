#ifndef __FLEXR_COMPONENTS_FREQUENCYMANAGER__
#define __FLEXR_COMPONENTS_FREQUENCYMANAGER__

#include <bits/stdc++.h>

namespace flexr
{
  namespace components
  {

    class FrequencyManager
    {
      private:
        uint32_t periodUs;
        uint32_t lastTs, exePeriod;
      public:
        FrequencyManager(int targetFrequency);
        void adjust();
    };

  }
}
#endif

