#include <iostream>
#include <flexr>

auto main() -> int
{
  std::clog << "[ Vulkan Renderer Example ]\n";

  raft::map pipeline;

  flexr::kernels::VulkanRenderer renderer {"VulkanRenderer"};
  renderer.activateOutPortAsLocal<flexr::kernels::VulkanRendererMsgType>("out_msg");
  renderer.setLogger("vulkan_renderer_logger", "vulkan_renderer.log");

  flexr::kernels::ImageSink sink {"ImageSink"};
  sink.activateInPortAsLocal<flexr::kernels::ImageSinkMsgType>("in_msg");
  sink.setLogger("image_sink_logger", "image_sink.log");

  pipeline += renderer >> sink;
  pipeline.exe();
}
