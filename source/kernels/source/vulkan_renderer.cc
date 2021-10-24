#include "kernels/source/vulkan_renderer.h"
#include "utils/msg_sending_functions.h"

namespace flexr::kernels
{

VulkanRenderer::VulkanRenderer(const std::string& id)
  : FleXRKernel {id}
{
  setName("VulkanRenderer");
  portManager.registerOutPortTag("out_msg", utils::sendLocalFrameCopy, nullptr, nullptr);

  renderer.emplace(256, 256);
}

auto VulkanRenderer::run() -> raft::kstatus
{
  renderer->Render();
  const auto& frame = renderer->GetRenderFrame();

  auto msg = portManager.getOutputPlaceholder<VulkanRendererMsgType>("out_msg");
  msg->data = {frame.width, frame.height, frame.channels, frame.data};
  std::strcpy(msg->tag, "rendered_frame");
  msg->seq = 0; // TODO
  msg->ts  = getTsNow();
  debug_print("Rendered frame");
  portManager.sendOutput<VulkanRendererMsgType>("out_msg", msg);
  return raft::stop;
}

} // namespace flexr::kernels
