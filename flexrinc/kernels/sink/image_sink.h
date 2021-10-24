#pragma once

#include <string>

#include "kernels/kernel.h"

namespace flexr::kernels
{
using ImageSinkMsgType = types::Message<std::string>;

class ImageSink : public FleXRKernel
{
public:
  ImageSink(const std::string& id);

  auto run() -> raft::kstatus override;

private:
};

} // namespace flexr::kernels
