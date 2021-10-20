#include <kernels/sink/image_sink.h>

namespace flexr::kernels
{

ImageSink::ImageSink(const std::string& id)
  : FleXRKernel {id}
{
  setName("ImageSink");
  portManager.registerInPortTag("in", components::PortDependency::BLOCKING, nullptr);
}

auto ImageSink::run() -> raft::kstatus
{
  const auto input = portManager.getInput<ImageSinkMsgType>("in");
  debug_print("Received: %s", input->data.c_str());
  portManager.freeInput("in", input);
  return raft::proceed;
}

} // namespace flexr::kernels
