#include <filesystem>
#include <optional>
#include <type_traits>

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#define VMA_STATIC_VULKAN_FUNCTIONS   0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS  0
#include <vk_mem_alloc.h>

#include <renderdoc_app.h>

#include <fx/gltf.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vulkan_utils
{

struct Context
{
  vk::Instance        instance;
  vk::PhysicalDevice  physicalDevice;
  vk::Device          device;
  vk::Queue           queue; // TODO: Have separate queues for graphics, compute, and transfers

  uint32_t            queueFamilyIndex {0};

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
  vk::Format          format;
  vk::ImageUsageFlags usage;
  vk::ImageLayout     initialLayout;
  vk::Image           image;
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

// TODO: Replace with glm vector types
template <typename T, int N>
struct GLTFBuffer
{
  using value_type = T;
  static constexpr auto components = N;

  const T* data   {nullptr}; // TODO: Use C++20 std::span
  uint32_t count  {0};
  uint32_t stride {0};
};

struct DrawInfo
{
  uint32_t indexCount   {0};
  uint32_t indexOffset  {0};
  uint32_t vertexOffset {0};
};

struct Scene
{
  struct Vertex
  {
    float px {0}, py {0}, pz {0}; // position
    float nx {0}, ny {0}, nz {0}; // normal
  };

  using Index = uint16_t;

  std::vector<DrawInfo> drawInfos;
  std::vector<Index>    indices;
  std::vector<Vertex>   vertices;
};

auto CreateContext() -> Context;

auto CreateBuffer(
  const Context& context,
  vk::DeviceSize size,
  VmaMemoryUsage usage) -> Buffer;

auto CreateShaderModule(
  const Context&                context,
  const std::filesystem::path&  path) -> vk::ShaderModule;

auto CreateAttachmentDescription(
  const Image& image,
  vk::ImageLayout finalLayout = vk::ImageLayout::eGeneral) -> vk::AttachmentDescription;

auto SubmitWork(
  const Context&    context,
  vk::CommandBuffer cmdBuf,
  bool              block=true) -> void;

// TODO: Add destruction methods
//auto DestroyContext(Context&& context) -> void;
// auto DestroyBuffer(Buffer&& buffer) -> void;
// auto DestroyImage(Image&& image) -> void;

auto LoadScene(const std::filesystem::path& path) -> Scene;

auto DrawScene(const Scene& scene, vk::CommandBuffer cmdBuf) -> void;

template <typename T>
inline auto SizeInBytes(T&& container)
{
  return std::forward<T>(container).size() * sizeof(std::forward<T>(container)[0]);
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

template <typename T, int N>
auto GetGLTFBuffer(const fx::gltf::Document& scene, int attribIndex) -> GLTFBuffer<T, N>
{
  const auto& accessor   = scene.accessors[attribIndex];
  const auto& bufferView = scene.bufferViews[accessor.bufferView];
  const auto& buffer     = scene.buffers[bufferView.buffer];

  const auto data = reinterpret_cast<const T*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
  const uint32_t stride = bufferView.byteStride ? (bufferView.byteStride / sizeof(T)) : N;
  return {data, accessor.count, stride};
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
  Renderer(
    int width,
    int height,
    const std::filesystem::path& assetPath);

  ~Renderer();

  auto Render() -> void;

  auto GetRenderFrame() const { return m_frame; }

private:
  RENDERDOC_API_1_4_2* m_renderdoc {nullptr};

  vk::Extent3D        m_extent;
  vk::CommandPool     m_commandPool;
  vk::RenderPass      m_renderPass;
  vk::Framebuffer     m_framebuffer;
  vk::DescriptorSet   m_descSet;
  vk::PipelineLayout  m_pipelineLayout;
  vk::Pipeline        m_pipeline;

  vku::Scene          m_scene;
  vku::Context        m_context;
  vku::Buffer         m_uniformBuffer;
  vku::Buffer         m_vertexBuffer;
  vku::Buffer         m_indexBuffer;
  vku::Image          m_colorImage;
  vku::Image          m_depthImage;
  vku::Image          m_copyImage;

  RenderFrame         m_frame;

  struct FrameData
  {
    glm::mat4 mvpMat;
    glm::mat4 modelMat;
    glm::mat4 normalMat;
  };
  FrameData           m_frameData;
};
