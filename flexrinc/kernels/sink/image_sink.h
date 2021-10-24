#pragma once

#include <string>

#include "kernels/kernel.h"

namespace flexr::kernels
{
using ImageSinkMsgType = types::Message<types::Image>;

class ImageSink : public FleXRKernel
{
public:
  ImageSink(const std::string& id);

  auto run() -> raft::kstatus override;

private:
};

} // namespace flexr::kernels
