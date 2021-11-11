#include "kernels/source/renderer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#ifndef debug_print
#define debug_print(...) do { \
  fprintf(stderr, "\033[1;31m[DEBUG] \033[0;32m[FUNC] %s \033[0m", __PRETTY_FUNCTION__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
} while (0)
#endif

auto main(int argc, char** argv) -> int
{
  constexpr auto defaultAssetPath = "assets/Sponza/Sponza.gltf";
  const auto assetPath = argc > 1 ? argv[1] : defaultAssetPath;

  Renderer renderer {512, 512, assetPath};
  renderer.Render();
  const auto& frame = renderer.GetRenderFrame();
  const auto stride = frame.width * frame.channels;
  stbi_write_png("result.png", frame.width, frame.height, frame.channels, frame.data.data(), stride);
  debug_print("Saved image");
}
