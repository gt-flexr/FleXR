#include <filesystem>
#include <optional>
#include <type_traits>

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

struct Resource
{
  VmaAllocation allocation;
};

template <typename T>
inline constexpr auto is_resource_v = std::is_base_of_v<Resource, T>;

struct Buffer : Resource
{
  vk::Buffer buffer;
};

struct Image : Resource
{
  vk::Image image;
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

auto CreateBuffer(
  const Context& context,
  vk::DeviceSize size,
  VmaMemoryUsage usage) -> Buffer;

auto CreateShaderModule(
  const Context&                context,
  const std::filesystem::path&  path) -> vk::ShaderModule;

auto SubmitWork(
  const Context&    context,
  vk::CommandBuffer cmdBuf,
  bool              block=true) -> void;

// TODO: Add destruction methods
//auto DestroyContext(Context&& context) -> void;
// auto DestroyBuffer(Buffer&& buffer) -> void;
// auto DestroyImage(Image&& image) -> void;

template <typename T>
inline auto SizeInBytes(T&& container)
{
  return std::forward<T>(container).size() * sizeof(std::forward<T>(container))[0];
}

template <typename Src, typename Dst>
auto CopyBuffer(const Context& context, Src&& container, const Dst& object)
  -> std::enable_if_t<is_resource_v<Dst>>
{
  char* data {nullptr};
  vmaMapMemory(context.allocator, object.allocation, reinterpret_cast<void**>(&data));
  std::memcpy(data, std::forward<Src>(container).data(), SizeInBytes(std::forward<Src>(container)));
  vmaUnmapMemory(context.allocator, object.allocation);
}

template <typename Src, typename Dst>
auto CopyBuffer(const Context& context, const Src& object, Dst&& container)
  -> std::enable_if_t<is_resource_v<Src>>
{
  char* data {nullptr};
  vmaMapMemory(context.allocator, object.allocation, reinterpret_cast<void**>(&data));
  std::memcpy(std::forward<Dst>(container).data(), data, SizeInBytes(std::forward<Dst>(container)));
  vmaUnmapMemory(context.allocator, object.allocation);
}

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
  vk::DescriptorSet m_descSet;
  vk::PipelineLayout m_pipelineLayout;
  vk::Pipeline m_pipeline;

  vku::Context    m_context;
  vku::Buffer     m_vertexStorageBuffer;
  vku::Image      m_framebufferImage;
  vku::Image      m_copyImage;

  RenderFrame     m_frame;
};
