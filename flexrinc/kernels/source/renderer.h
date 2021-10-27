#include <optional>

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
  vk::Queue           queue; // TODO: Have separate queues for graphics, compute, and transfers

  VmaAllocator        allocator;
};

struct Image
{
  VmaAllocation allocation;
  vk::Image     image;

  std::optional<vk::ImageView> view;
};

class ImageBuilder
{
public:
  ImageBuilder(vk::Extent3D extent, vk::Format, VmaMemoryUsage usage);

  auto SetUsage(vk::ImageUsageFlags usage) -> ImageBuilder&;
  auto SetTiling(vk::ImageTiling tiling)   -> ImageBuilder&;

  auto Build(const Context& context) const -> Image;

private:

  vk::ImageCreateInfo imageCI;
  VmaMemoryUsage      usage;
};

auto CreateContext() -> Context;

auto SubmitWork(const Context& context, vk::CommandBuffer cmdBuf, bool block=true) -> void;

// TODO: Add destruction methods
//auto DestroyContext(Context&& context) -> void;
// auto DestroyImage(Image&& image) -> void;

} // namespace vulkan_utils

namespace vku = vulkan_utils;

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

private:
  RENDERDOC_API_1_4_2* m_renderdoc {nullptr};

  vk::Extent3D    m_extent;
  vk::CommandPool m_commandPool;
  vk::RenderPass  m_renderPass;
  vk::Framebuffer m_framebuffer;

  vku::Context    m_context;
  vku::Image      m_framebufferImage;
  vku::Image      m_copyImage;

  RenderFrame     m_frame;
};
