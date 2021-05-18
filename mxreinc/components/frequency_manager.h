#ifndef __MXRE_COMPONENTS_FREQUENCYMANAGER__
#define __MXRE_COMPONENTS_FREQUENCYMANAGER__

#include <bits/stdc++.h>

namespace mxre
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

