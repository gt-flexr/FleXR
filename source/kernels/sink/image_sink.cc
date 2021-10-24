#include "kernels/sink/image_sink.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace flexr::kernels
{

ImageSink::ImageSink(const std::string& id)
  : FleXRKernel {id}
{
  setName("ImageSink");
  portManager.registerInPortTag("in_msg", components::PortDependency::BLOCKING, nullptr);
}

auto ImageSink::run() -> raft::kstatus
{
  const auto msg = portManager.getInput<ImageSinkMsgType>("in_msg");
  const auto& image = msg->data;
  stbi_write_bmp("result.bmp", image.width, image.height, image.channels, image.data.data());
  debug_print("Saved image");
  portManager.freeInput("in_msg", msg);
  return raft::proceed;
}

} // namespace flexr::kernels
