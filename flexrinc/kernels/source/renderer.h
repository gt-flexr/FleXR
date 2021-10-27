#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#define VMA_STATIC_VULKAN_FUNCTIONS   0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS  0
#include <vk_mem_alloc.h>

#include <renderdoc_app.h>

namespace vulkan_utils
{

struct Context
{
  vk::Instance        instance;
  vk::PhysicalDevice  physicalDevice;
  vk::Device          device;
  vk::Queue           queue;

  VmaAllocator        allocator;
};

auto CreateContext() -> Context;

//auto DestroyContext(vku::Context&& context) -> void; // TODO

} // namespace vulkan_utils

namespace vku = vulkan_utils;

struct Image
{
  vk::Image     image;
  vk::ImageView view;
  VmaAllocation allocation;
};

struct RenderFrame
{
  int width    {0};
  int height   {0};
  int channels {0};
  std::vector<char> data;
};

class Renderer
{
public:
  Renderer(int width, int height);

  ~Renderer();

  auto Render() -> void;

  auto GetRenderFrame() const { return m_frame; }

  auto SubmitWork(vk::CommandBuffer commandBuffer) -> void;

private:
  RENDERDOC_API_1_4_2* m_renderdoc {nullptr};

  vk::Extent3D        m_extent;
  vk::CommandPool     m_commandPool;
  vk::RenderPass      m_renderPass;
  vk::Framebuffer     m_framebuffer;

  vku::Context        m_context;

  Image               m_framebufferImage;
  Image               m_copyImage;

  RenderFrame         m_frame;
};
