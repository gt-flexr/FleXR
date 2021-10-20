#ifndef __FLEXR_VULKAN_RENDERER__
#define __FLEXR_VULKAN_RENDERER__

#include <string>

#include "kernels/kernel.h"

namespace flexr::kernels
{
using VulkanRendererMsgType = types::Message<std::string>;

class VulkanRenderer : public FleXRKernel
{
public:
  VulkanRenderer(const std::string& id);

  auto run() -> raft::kstatus override;

private:
};

} // namespace flexr::kernels

#endif
