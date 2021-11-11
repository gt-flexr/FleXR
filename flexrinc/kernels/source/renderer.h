#include <filesystem>
#include <functional>
#include <optional>
#include <type_traits>
#include <variant>

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
  vk::CommandPool     commandPool; // Internally used for immediate command buffer submits

  uint32_t            queueFamilyIndex {0};

  VmaAllocator        allocator;
};

struct VmaResource
{
  VmaAllocation allocation;
};

template <typename T>
inline constexpr auto is_vmaresource_v = std::is_base_of_v<VmaResource, T>;

struct Buffer : VmaResource
{
  vk::Buffer buffer;
};

struct Image : VmaResource
{
  vk::Format            format;
  vk::ImageUsageFlags   usage;
  vk::ImageAspectFlags  aspect;
  vk::Image             image;
  vk::Sampler           sampler;

  std::vector<vk::ImageView> views;
};

class ImageBuilder
{
public:
  ImageBuilder(vk::Extent3D extent, vk::Format, VmaMemoryUsage vmaUsage);
  ImageBuilder(vk::Extent3D extent, vk::Format, vk::ArrayProxy<std::filesystem::path> paths);

  auto SetUsage(vk::ImageUsageFlags usage) -> ImageBuilder&;
  auto SetTiling(vk::ImageTiling tiling)   -> ImageBuilder&;
  auto SetLayers(uint32_t layers)          -> ImageBuilder&;

  auto Build(const Context& context) const -> Image;

private:
  vk::ImageCreateInfo imageCI;
  VmaMemoryUsage      vmaUsage;

  std::vector<std::filesystem::path> paths;
};

struct Descriptor
{
  vk::DescriptorType type;
  uint32_t count {1};
};

struct DescriptorBinding
{
  Descriptor  descriptor;
  uint32_t    binding {0};
};

struct DescriptorSet
{
  vk::DescriptorSet       set;
  vk::DescriptorSetLayout layout;
  std::vector<DescriptorBinding> bindings;
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
  uint32_t imageId      {0};
};

struct Scene
{
  struct Vertex
  {
    float px {0}, py {0}, pz {0}; // position
    float nx {0}, ny {0}, nz {0}; // normal
    float tu {0}, tv {0};         // texcoord
  };

  using Index = uint16_t;

  std::vector<DrawInfo> drawInfos;
  std::vector<Index>    indices;
  std::vector<Vertex>   vertices;

  Image imageArray;
};

template <typename T>
struct PushConstant
{
  T        value;
  uint32_t size   {sizeof(T)};
  uint32_t offset {0};
  vk::ShaderStageFlags stageFlags {vk::ShaderStageFlagBits::eAllGraphics};
};

auto CreateContext() -> Context;

auto CreateBuffer(
  const Context&        context,
  vk::DeviceSize        size,
  vk::BufferUsageFlags  usage,
  VmaMemoryUsage        memoryUsage
) -> Buffer;

auto CreateAttachmentDescription(
  const Image& image,
  vk::ImageLayout finalLayout = vk::ImageLayout::eGeneral
) -> vk::AttachmentDescription;

auto CreateDescriptorSet(
  const Context& context,
  vk::ArrayProxy<const Descriptor> descriptors
) -> DescriptorSet;

auto CreatePipelineLayout(
  const Context&        context,
  const DescriptorSet&  descSet
) -> vk::PipelineLayout;

template <typename T>
auto CreatePipelineLayout(
  const Context&          context,
  const DescriptorSet&    descSet,
  const PushConstant<T>&  pushConstant
) -> vk::PipelineLayout
{
  vk::PushConstantRange pcr;
  pcr.stageFlags  = pushConstant.stageFlags;
  pcr.offset      = pushConstant.offset;
  pcr.size        = pushConstant.size;

  vk::PipelineLayoutCreateInfo ci;
  ci.setLayoutCount         = 1;
  ci.pSetLayouts            = &descSet.layout;
  ci.pushConstantRangeCount = 1;
  ci.pPushConstantRanges    = &pcr;
  return context.device.createPipelineLayout(ci);
}

auto CreateShaderModule(
  const Context&                context,
  const std::filesystem::path&  path
) -> vk::ShaderModule;

auto CreateGraphicsPipelineShaderStages(
  const Context&                context,
  const std::filesystem::path&  vertexShaderPath,
  const std::filesystem::path&  fragmentShaderPath
) -> std::array<vk::PipelineShaderStageCreateInfo, 2>;

auto BindDescriptorSet(
  vk::CommandBuffer     cmdBuf,
  vk::PipelineLayout    pipelineLayout,
  const DescriptorSet&  descSet,
  vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics
) -> void;

template <typename T>
auto BindPushConstant(
  vk::CommandBuffer cmdBuf,
  vk::PipelineLayout pipelineLayout,
  const PushConstant<T>& pushConstant
) -> void
{
  cmdBuf.pushConstants(
    pipelineLayout,
    pushConstant.stageFlags,
    pushConstant.offset,
    pushConstant.size,
    reinterpret_cast<const void*>(&pushConstant.value));
}

auto SetImageLayout(
  vk::CommandBuffer cmdBuf,
  const Image& image,
  vk::ImageLayout oldLayout,
  vk::ImageLayout newLayout,
  vk::ImageSubresourceRange subresourceRange,
  vk::PipelineStageFlags srcStageMask = vk::PipelineStageFlagBits::eAllCommands,
  vk::PipelineStageFlags dstStageMask = vk::PipelineStageFlagBits::eAllCommands
) -> void;

using submit_function_type = std::function<void(vk::CommandBuffer)>;

auto SubmitImmediate(
  const Context& context,
  submit_function_type function,
  bool block=true) -> void;

auto SubmitWork(
  const Context&    context,
  vk::CommandBuffer cmdBuf,
  bool              block=true) -> void;

// TODO: Add destruction methods
//auto DestroyContext(Context&& context) -> void;
// auto DestroyBuffer(Buffer&& buffer) -> void;
// auto DestroyImage(Image&& image) -> void;

auto LoadScene(const Context& context, const std::filesystem::path& path) -> Scene;

auto DrawScene(
  vk::CommandBuffer cmdBuf,
  vk::PipelineLayout pipelineLayout, // TODO: Move into scene class
  const Scene& scene
) -> void;

template <typename T>
auto UpdateDescriptorSet(
  const Context&        context,
  const DescriptorSet&  descSet,
  uint32_t              index,
  const T&              object
) -> std::enable_if_t<is_vmaresource_v<T>>
{
  const auto& binding = descSet.bindings[index];

  vk::WriteDescriptorSet writeDescSet;
  writeDescSet.dstSet          = descSet.set;
  writeDescSet.dstBinding      = binding.binding;
  writeDescSet.descriptorCount = binding.descriptor.count;
  writeDescSet.descriptorType  = binding.descriptor.type;

  if constexpr (std::is_same_v<T, Image>)
  {
    const auto& image = object; // Alias
    std::vector<vk::DescriptorImageInfo> infos (image.views.size());
    std::transform(
      std::cbegin(image.views), std::cend(image.views),
      std::begin(infos),
      [&](const auto& v) -> vk::DescriptorImageInfo
      {
        return {image.sampler, v, vk::ImageLayout::eShaderReadOnlyOptimal}; // TODO: Parameterize image layout
      });
    writeDescSet.pImageInfo = infos.data();
    return context.device.updateDescriptorSets(writeDescSet, {});
  }

  if constexpr (std::is_same_v<T, Buffer>)
  {
    const auto& buffer = object; // Alias
    vk::DescriptorBufferInfo info {buffer.buffer, 0, VK_WHOLE_SIZE}; // TODO: Parameterize buffer offset and range
    writeDescSet.pBufferInfo = &info;
    return context.device.updateDescriptorSets(writeDescSet, {});
  }
}

template <typename T>
inline auto SizeInBytes(T&& container)
{
  return std::forward<T>(container).size() * sizeof(std::forward<T>(container)[0]);
}

template <typename Src, typename Dst>
auto CopyBuffer(const Context& context, Src&& container, const Dst& object)
  -> std::enable_if_t<is_vmaresource_v<Dst>>
{
  char* data {nullptr};
  vmaMapMemory(context.allocator, object.allocation, reinterpret_cast<void**>(&data));
  std::memcpy(data, std::forward<Src>(container).data(), SizeInBytes(std::forward<Src>(container)));
  vmaUnmapMemory(context.allocator, object.allocation);
}

template <typename Src, typename Dst>
auto CopyBuffer(const Context& context, const Src& object, Dst&& container)
  -> std::enable_if_t<is_vmaresource_v<Src>>
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

struct DrawPushConstant
{
  uint32_t imageId {0};
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
  vku::DescriptorSet  m_descSet;

  RenderFrame         m_frame;

  struct FrameData
  {
    glm::mat4 mvpMat;
    glm::mat4 modelMat;
    glm::mat4 normalMat;
  };
  FrameData           m_frameData;
};
