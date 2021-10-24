#pragma once

#include <string>
#include <optional>

#include "kernels/kernel.h"
#include "renderer.h"

namespace flexr::kernels
{
using VulkanRendererMsgType = types::Message<types::Image>;

class VulkanRenderer : public FleXRKernel
{
public:
  VulkanRenderer(const std::string& id);

  auto run() -> raft::kstatus override;

private:
  std::optional<Renderer> renderer;
};

} // namespace flexr::kernels
