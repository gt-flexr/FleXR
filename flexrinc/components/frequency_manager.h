#ifndef __FLEXR_COMPONENTS_FREQUENCYMANAGER__
#define __FLEXR_COMPONENTS_FREQUENCYMANAGER__

#include <bits/stdc++.h>

namespace flexr
{
  namespace components
  {

    /**
     * @brief Component to manage the frequency of kernel executions
     */
    class FrequencyManager
    {
      private:
        uint32_t periodUs;
        uint32_t lastTs, exePeriod;
      public:
        FrequencyManager(int targetFrequency = 200);
        void setFrequency(int targetFrequency);
        void adjust();
    };

  }
}
#endif

