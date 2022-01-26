#include <flexr_core/include/components/frequency_manager.h>
#include <flexr_core/include/defs.h>

namespace flexr
{
  namespace components
  {

    FrequencyManager::FrequencyManager(int targetFrequency)
    {
      setFrequency(targetFrequency);
    }


    void FrequencyManager::setFrequency(int targetFrequency)
    {
      if(targetFrequency == 0) periodUs = 5000;
      else periodUs = (uint32_t)(1000000/targetFrequency);

      lastTs = getTsUs();
    }


    void FrequencyManager::adjust()
    {
      exePeriod = getTsUs() - lastTs;
      if (exePeriod < periodUs) {
        std::this_thread::sleep_for(std::chrono::microseconds(periodUs - exePeriod));
      }
      lastTs = getTsUs();
    }

  }
}

