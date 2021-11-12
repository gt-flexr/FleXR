#include "kernels/source/renderer.h"

#include <array>
#include <algorithm>
#include <cmath>
#include <dlfcn.h> // For dlopen
#include <fstream>
#include <map>
#include <vector>
#include <iostream> // TODO: Remove this

#include <VkBootstrap.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>

#define VMA_ASSERT(expr) assert(expr)
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define ASSERT_THROW(status, ...) \
if (!(status)) { \
  debug_print(__VA_ARGS__); \
  throw; \
}

#ifndef debug_print
#define debug_print(...) do { \
  fprintf(stderr, "\033[1;31m[DEBUG] \033[0;32m[FUNC] %s \033[0m", __func__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n"); \
} while (0)
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace vulkan_utils
{

auto CreateContext() -> Context
{
  vk::DynamicLoader dl;
  const auto vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
  VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

  vkb::InstanceBuilder instanceBuilder;
  const auto instanceStatus = instanceBuilder
    .set_app_name("FleXR")
    .set_engine_name("VulkanRenderer")
    .set_headless(true)
    .require_api_version(1, 2)
    .enable_extension("VK_KHR_get_physical_device_properties2")
    .request_validation_layers()
    .use_default_debug_messenger()
    .build();
  ASSERT_THROW(instanceStatus,
    "Failed to create vulkan instance! %s", instanceStatus.error().message().c_str());

  const auto vkbInstance = instanceStatus.value();
  const auto instance = vkbInstance.instance;
  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

  vk::PhysicalDeviceFeatures features;
  features.samplerAnisotropy = true;

  vkb::PhysicalDeviceSelector physicalDeviceSelector {vkbInstance};
  const auto physicalDeviceStatus = physicalDeviceSelector
    .set_minimum_version(1, 2)
    .add_required_extension("VK_KHR_get_memory_requirements2")
    .add_required_extension("VK_KHR_bind_memory2")
    .add_required_extension("VK_KHR_maintenance1")
    .set_required_features(features)
    .select();
  ASSERT_THROW(physicalDeviceStatus,
    "Failed to select vulkan physical device! %s", physicalDeviceStatus.error().message().c_str());
  const auto vkbPhysicalDevice = physicalDeviceStatus.value();
  const auto physicalDevice = vk::PhysicalDevice {vkbPhysicalDevice.physical_device};

  const auto physicalDeviceProps = physicalDevice.getProperties();
  const std::string deviceName = physicalDeviceProps.deviceName;
  debug_print(
    "\nVulkan physical device:\n"
    "- Name:   %s\n"
    "- API:    %d.%d.%d\n"
    "- Driver: %d.%d.%d",
    deviceName.c_str(),
    VK_VERSION_MAJOR(physicalDeviceProps.apiVersion),
    VK_VERSION_MINOR(physicalDeviceProps.apiVersion),
    VK_VERSION_PATCH(physicalDeviceProps.apiVersion),
    VK_VERSION_MAJOR(physicalDeviceProps.driverVersion),
    VK_VERSION_MINOR(physicalDeviceProps.driverVersion),
    VK_VERSION_PATCH(physicalDeviceProps.driverVersion));

  vkb::DeviceBuilder deviceBuilder {vkbPhysicalDevice};
  const auto deviceStatus = deviceBuilder.build();
  ASSERT_THROW(deviceStatus,
    "Failed to create Vulkan device! %s", deviceStatus.error().message().c_str());
  const auto vkbDevice = deviceStatus.value();
  const auto device = vk::Device {vkbDevice.device};
  VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

  VmaAllocatorCreateInfo allocatorCI;
  allocatorCI.vulkanApiVersion = VK_MAKE_VERSION(1, 2, 0);
  allocatorCI.physicalDevice   = physicalDevice;
  allocatorCI.device           = device;
  allocatorCI.instance         = instance;

#define SET_FN(name) name = reinterpret_cast<PFN_##name>(VULKAN_HPP_DEFAULT_DISPATCHER.name)
  VmaVulkanFunctions fn;
  fn.SET_FN(vkGetPhysicalDeviceProperties);
  fn.SET_FN(vkGetPhysicalDeviceMemoryProperties);
  fn.SET_FN(vkAllocateMemory);
  fn.SET_FN(vkFreeMemory);
  fn.SET_FN(vkMapMemory);
  fn.SET_FN(vkUnmapMemory);
  fn.SET_FN(vkFlushMappedMemoryRanges);
  fn.SET_FN(vkInvalidateMappedMemoryRanges);
  fn.SET_FN(vkBindBufferMemory);
  fn.SET_FN(vkBindImageMemory);
  fn.SET_FN(vkGetBufferMemoryRequirements);
  fn.SET_FN(vkGetImageMemoryRequirements);
  fn.SET_FN(vkCreateBuffer);
  fn.SET_FN(vkDestroyBuffer);
  fn.SET_FN(vkCreateImage);
  fn.SET_FN(vkDestroyImage);
  fn.SET_FN(vkCmdCopyBuffer);
  fn.SET_FN(vkGetBufferMemoryRequirements2KHR);
  fn.SET_FN(vkGetImageMemoryRequirements2KHR);
  fn.SET_FN(vkBindBufferMemory2KHR);
  fn.SET_FN(vkBindImageMemory2KHR);
  fn.SET_FN(vkGetPhysicalDeviceMemoryProperties2KHR);
  allocatorCI.pVulkanFunctions = &fn;
#undef SET_FN

  VmaAllocator allocator {};
  const auto allocatorStatus = vmaCreateAllocator(&allocatorCI, &allocator);
  ASSERT_THROW(allocatorStatus == VK_SUCCESS,
    "Failed to create vulkan allocator! %d", allocatorStatus);

  const auto queueType = vkb::QueueType::graphics;
  const auto queueStatus = vkbDevice.get_queue(queueType);
  ASSERT_THROW(queueStatus,
    "Failed to get vulkan queue! %s", queueStatus.error().message().c_str());
  const auto queue = vk::Queue {queueStatus.value()};

  const auto queueFamilyIndex = vkbDevice.get_queue_index(queueType).value();

  const auto commandPool = device.createCommandPool({{}, queueFamilyIndex});

  return {instance, physicalDevice, device, queue, commandPool, queueFamilyIndex, allocator};
}

ImageBuilder::ImageBuilder(vk::Extent3D extent, vk::Format format, VmaMemoryUsage vmaUsage)
  : vmaUsage {vmaUsage}
{
  const auto tiling =
    (vmaUsage == VMA_MEMORY_USAGE_CPU_TO_GPU) || (vmaUsage == VMA_MEMORY_USAGE_GPU_TO_CPU) ?
    vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal;

  const auto layout = tiling == vk::ImageTiling::eLinear ?
    vk::ImageLayout::ePreinitialized : vk::ImageLayout::eUndefined;

  imageCI.imageType     = vk::ImageType::e2D;
  imageCI.format        = format;
  imageCI.extent        = extent;
  imageCI.arrayLayers   = 1;
  imageCI.mipLevels     = 1;
  imageCI.initialLayout = layout;
  imageCI.samples       = vk::SampleCountFlagBits::e1;
  imageCI.tiling        = tiling;
  imageCI.usage         = vk::ImageUsageFlagBits::eColorAttachment;
}

ImageBuilder::ImageBuilder(vk::Extent3D extent, vk::Format format, vk::ArrayProxy<std::filesystem::path> paths)
  : ImageBuilder {extent, format, VMA_MEMORY_USAGE_CPU_TO_GPU}
{
  this->imageCI.arrayLayers = paths.size();
  this->paths.assign(std::cbegin(paths), std::cend(paths));
}

auto ImageBuilder::SetUsage(vk::ImageUsageFlags usage) -> ImageBuilder&
{
  imageCI.usage = usage;
  return *this;
}

auto ImageBuilder::SetTiling(vk::ImageTiling tiling) -> ImageBuilder&
{
  imageCI.tiling = tiling;
  return *this;
}

auto ImageBuilder::SetArrayLayers(uint32_t layers) -> ImageBuilder&
{
  imageCI.arrayLayers = layers;
  return *this;
}

auto ImageBuilder::SetMipLevels(uint32_t levels) -> ImageBuilder&
{
  // Assume automatic mipmap generation
  if (levels == sAllMipLevels)
    levels = CalculateMipLevels({imageCI.extent.width, imageCI.extent.height});
  imageCI.usage     |= vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;
  imageCI.mipLevels  = levels;
  return *this;
}

auto ImageBuilder::Build(const Context& context) const -> Image
{
  Image result;
  result.format       = imageCI.format;
  result.extent       = imageCI.extent;
  result.mipLevels    = imageCI.mipLevels;
  result.arrayLayers  = imageCI.arrayLayers;
  result.usage        = imageCI.usage;
  result.aspect       =
    imageCI.usage == vk::ImageUsageFlagBits::eDepthStencilAttachment ?
    vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;

  VmaAllocationCreateInfo allocationCI {};
  allocationCI.usage = vmaUsage;

  const auto imageStatus = vmaCreateImage(
    context.allocator,
    reinterpret_cast<const VkImageCreateInfo*>(&imageCI),
    &allocationCI,
    reinterpret_cast<VkImage*>(&result.image),
    &result.allocation,
    nullptr);
  ASSERT_THROW(imageStatus == VK_SUCCESS, "Failed to create vulkan image! %d", imageStatus);

  // All images have default sampler
  // TODO: Add support for other sampler types and configuring with sampler
  result.sampler = CreateSampler(
    context, vk::Filter::eLinear, vk::Filter::eLinear, imageCI.mipLevels);

  constexpr auto validImageViewUsageFlags =
    vk::ImageUsageFlagBits::eSampled                |
    vk::ImageUsageFlagBits::eStorage                |
    vk::ImageUsageFlagBits::eColorAttachment        |
    vk::ImageUsageFlagBits::eDepthStencilAttachment |
    vk::ImageUsageFlagBits::eInputAttachment        |
    vk::ImageUsageFlagBits::eTransientAttachment;

  // Only create image views if possible and one view per array layer for image arrays
  if (imageCI.usage & validImageViewUsageFlags)
  {
    result.views.resize(imageCI.arrayLayers);
    auto layerCounter = 0U;
    std::generate(
      std::begin(result.views), std::end(result.views),
      [&]() mutable -> vk::ImageView
      {
        vk::ImageViewCreateInfo imageViewCI;
        imageViewCI.image    = result.image;
        imageViewCI.viewType = vk::ImageViewType::e2D;
        imageViewCI.format   = imageCI.format; // TODO: Support aliased formats
        imageViewCI.subresourceRange = {result.aspect, 0, imageCI.mipLevels, layerCounter++, 1};
        return context.device.createImageView(imageViewCI);
      });
  }

  vku::SubmitImmediate(context, [&](vk::CommandBuffer cmdBuf)
  {
    vku::SetImageLayout(
      cmdBuf, result,
      imageCI.initialLayout, vk::ImageLayout::eGeneral,
      {result.aspect, 0, imageCI.mipLevels, 0, imageCI.arrayLayers});
  });

  if (paths.size()) LoadImage(context, result, paths);

  if (imageCI.mipLevels > 1)
    GenerateMipmaps(context, result, vk::ImageLayout::eShaderReadOnlyOptimal);

  return result;
}

auto CreateBuffer(
  const Context&        context,
  vk::DeviceSize        size,
  vk::BufferUsageFlags  usage,
  VmaMemoryUsage        memoryUsage) -> Buffer
{
  vk::BufferCreateInfo bufferCI;
  bufferCI.size  = size;
  bufferCI.usage = usage;

  VmaAllocationCreateInfo allocationCI {};
  allocationCI.usage = memoryUsage;

  VkBuffer buffer;
  VmaAllocation allocation {};
  const auto bufferStatus = vmaCreateBuffer(
    context.allocator,
    reinterpret_cast<const VkBufferCreateInfo*>(&bufferCI),
    &allocationCI,
    &buffer,
    &allocation,
    nullptr);
  ASSERT_THROW(bufferStatus == VK_SUCCESS, "Failed to create vulkan buffer! %d", bufferStatus);

  return {allocation, buffer};
}

auto CreateSampler(
  const Context&  context,
  vk::Filter      magFilter,
  vk::Filter      minFilter,
  uint32_t        mipLevels
) -> vk::Sampler
{
  vk::SamplerCreateInfo ci;
  ci.magFilter         = magFilter;
  ci.minFilter         = minFilter;
  ci.mipmapMode        = vk::SamplerMipmapMode::eLinear;
  ci.mipLodBias        = 0;
  ci.anisotropyEnable  = true;
  ci.maxAnisotropy     = 16;
  ci.minLod            = 0;
  ci.maxLod            = mipLevels;
  return context.device.createSampler(ci);
}

auto CreateAttachmentDescription(
  const Image& image,
  vk::ImageLayout finalLayout) -> vk::AttachmentDescription
{
  const auto isDepthAttachment = image.usage == vk::ImageUsageFlagBits::eDepthStencilAttachment;

  vk::AttachmentDescription desc;
  desc.format        = image.format;
  desc.samples       = vk::SampleCountFlagBits::e1;
  desc.loadOp        = vk::AttachmentLoadOp::eClear; // TODO: eLoad for textures
  desc.storeOp       = isDepthAttachment ? vk::AttachmentStoreOp::eDontCare : vk::AttachmentStoreOp::eStore;
  desc.initialLayout = vk::ImageLayout::eUndefined; // TODO: Verify this
  desc.finalLayout   = isDepthAttachment ? vk::ImageLayout::eDepthStencilAttachmentOptimal : finalLayout;
  return desc;
}

auto CreateDescriptorSet(
  const Context& context,
  vk::ArrayProxy<const Descriptor> descriptors) -> DescriptorSet
{
  const auto numDescriptors = descriptors.size();

  std::vector<DescriptorBinding> bindings (numDescriptors);
  auto bindingCounter = 0U;
  std::transform(
    std::cbegin(descriptors), std::cend(descriptors),
    std::begin(bindings),
    [&](const auto& d) mutable -> DescriptorBinding
    {
      return {d, bindingCounter++};
    });

  std::vector<vk::DescriptorSetLayoutBinding> layoutBindings (numDescriptors);
  std::transform(
    std::cbegin(bindings), std::cend(bindings),
    std::begin(layoutBindings),
    [&](const auto& b) -> vk::DescriptorSetLayoutBinding
    {
      return {b.binding, b.descriptor.type, b.descriptor.count, vk::ShaderStageFlagBits::eAllGraphics};
    });

  vk::DescriptorSetLayoutCreateInfo descSetLayoutCI;
  descSetLayoutCI.bindingCount  = layoutBindings.size();
  descSetLayoutCI.pBindings     = layoutBindings.data();
  const auto descSetLayout = context.device.createDescriptorSetLayout(descSetLayoutCI);

  std::vector<vk::DescriptorPoolSize> sizes (numDescriptors);
  std::transform(
    std::cbegin(descriptors), std::cend(descriptors),
    std::begin(sizes),
    [&](const auto& d) -> vk::DescriptorPoolSize
    {
      return {d.type, d.count};
    });

  vk::DescriptorPoolCreateInfo descPoolCI;
  descPoolCI.maxSets        = 1;
  descPoolCI.poolSizeCount  = sizes.size();
  descPoolCI.pPoolSizes     = sizes.data();
  const auto descPool = context.device.createDescriptorPool(descPoolCI);

  vk::DescriptorSetAllocateInfo descSetAI;
  descSetAI.descriptorPool      = descPool;
  descSetAI.descriptorSetCount  = 1;
  descSetAI.pSetLayouts         = &descSetLayout;
  const auto descSet = context.device.allocateDescriptorSets(descSetAI)[0];

  return {descSet, descSetLayout, {std::cbegin(bindings), std::cend(bindings)}};
}

auto CreatePipelineLayout(
  const Context&        context,
  const DescriptorSet&  descSet) -> vk::PipelineLayout
{
  vk::PipelineLayoutCreateInfo ci;
  ci.setLayoutCount = 1;
  ci.pSetLayouts    = &descSet.layout;
  return context.device.createPipelineLayout(ci);
}

auto CreateShaderModule(
  const Context&                context,
  const std::filesystem::path&  path
) -> vk::ShaderModule
{
  std::basic_ifstream<char> file {path, std::ios::binary};
  ASSERT_THROW(file.is_open(), "Failed to open SPIRV shader! %s", path.c_str());
  const std::vector<char> data {std::istreambuf_iterator<char> {file}, {}};

  vk::ShaderModuleCreateInfo shaderModuleCI;
  shaderModuleCI.codeSize = data.size();
  shaderModuleCI.pCode    = reinterpret_cast<const uint32_t*>(data.data());
  return context.device.createShaderModule(shaderModuleCI);
}

auto CreateGraphicsPipelineShaderStages(
  const Context&                context,
  const std::filesystem::path&  vertexShaderPath,
  const std::filesystem::path&  fragmentShaderPath
) -> std::array<vk::PipelineShaderStageCreateInfo, 2>
{
  std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;
  shaderStages[0].stage  = vk::ShaderStageFlagBits::eVertex;
  shaderStages[0].pName  = "main";
  shaderStages[0].module = vku::CreateShaderModule(context, vertexShaderPath);
  shaderStages[1].stage  = vk::ShaderStageFlagBits::eFragment;
  shaderStages[1].pName  = "main";
  shaderStages[1].module = vku::CreateShaderModule(context, fragmentShaderPath);
  return shaderStages;
}

auto BindDescriptorSet(
  vk::CommandBuffer     cmdBuf,
  vk::PipelineLayout    pipelineLayout,
  const DescriptorSet&  descSet,
  vk::PipelineBindPoint bindPoint) -> void
{
  // TODO: SUpport multiple descriptor sets
  // TODO: Support push constants
  cmdBuf.bindDescriptorSets(bindPoint, pipelineLayout, 0, descSet.set, {});
}

auto SetImageLayout(
  vk::CommandBuffer cmdBuf,
  const Image& image,
  vk::ImageLayout oldLayout,
  vk::ImageLayout newLayout,
  vk::ImageSubresourceRange subresourceRange,
  vk::PipelineStageFlags srcStageMask,
  vk::PipelineStageFlags dstStageMask
) -> void
{
  vk::ImageMemoryBarrier barrier;
  barrier.oldLayout        = oldLayout;
  barrier.newLayout        = newLayout;
  barrier.image            = image.image;
  barrier.subresourceRange = subresourceRange;

  barrier.srcAccessMask = [&]() -> vk::AccessFlags
  {
    switch (oldLayout)
    {
      default:                                              [[fallthrough]];
      case vk::ImageLayout::eUndefined:                     return {};
      case vk::ImageLayout::ePreinitialized:                return vk::AccessFlagBits::eHostWrite;
      case vk::ImageLayout::eColorAttachmentOptimal:        return vk::AccessFlagBits::eColorAttachmentWrite;
      case vk::ImageLayout::eDepthStencilAttachmentOptimal: return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      case vk::ImageLayout::eTransferSrcOptimal:            return vk::AccessFlagBits::eTransferRead;
      case vk::ImageLayout::eTransferDstOptimal:            return vk::AccessFlagBits::eTransferWrite;
      case vk::ImageLayout::eShaderReadOnlyOptimal:         return vk::AccessFlagBits::eShaderRead;
    };
  }();

  barrier.dstAccessMask = [&]() -> vk::AccessFlags
  {
    switch (newLayout)
    {
      default:                                              return {};
      case vk::ImageLayout::eTransferDstOptimal:            return vk::AccessFlagBits::eTransferWrite;
      case vk::ImageLayout::eTransferSrcOptimal:            return vk::AccessFlagBits::eTransferRead;
      case vk::ImageLayout::eColorAttachmentOptimal:        return vk::AccessFlagBits::eColorAttachmentWrite;
      case vk::ImageLayout::eDepthStencilAttachmentOptimal: return vk::AccessFlagBits::eDepthStencilAttachmentWrite | barrier.dstAccessMask;
      case vk::ImageLayout::eShaderReadOnlyOptimal:
        if (barrier.srcAccessMask == vk::AccessFlags {})
        {
          barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
        }
        return vk::AccessFlagBits::eShaderRead;
    };
  }();

  cmdBuf.pipelineBarrier(srcStageMask, dstStageMask, {}, {}, {}, barrier);
}

auto SubmitImmediate(
  const Context& context,
  submit_function_type function,
  bool block) -> void
{
  const auto cmdBuf = context.device.allocateCommandBuffers({
    context.commandPool, vk::CommandBufferLevel::ePrimary, 1})[0];

  cmdBuf.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  function(cmdBuf);
  cmdBuf.end();

  SubmitWork(context, cmdBuf, block);
}

auto SubmitWork(const Context& context, vk::CommandBuffer cmdBuf, bool block) -> void
{
  vk::SubmitInfo submitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &cmdBuf;

  const auto fence = context.device.createFence({});
  context.queue.submit(submitInfo, fence);
  const auto status = context.device.waitForFences(
    fence, true, std::numeric_limits<uint64_t>::max());
  ASSERT_THROW(status == vk::Result::eSuccess, "Failed to wait for vulkan fence!");
  context.device.destroyFence(fence);

  if (block) context.device.waitIdle();
}

auto CalculateDataTypeSize(const fx::gltf::Accessor& accessor) -> uint32_t
{
  const auto elementSize = [&]()
  {
    switch (accessor.componentType)
    {
      case fx::gltf::Accessor::ComponentType::Byte:
      case fx::gltf::Accessor::ComponentType::UnsignedByte:
        return 1;
      case fx::gltf::Accessor::ComponentType::Short:
      case fx::gltf::Accessor::ComponentType::UnsignedShort:
        return 2;
      case fx::gltf::Accessor::ComponentType::Float:
      case fx::gltf::Accessor::ComponentType::UnsignedInt:
        return 4;
    }
    return 0; // Only to suppress compiler warning
  }();

  switch (accessor.type)
  {
    case fx::gltf::Accessor::Type::Scalar:
      return 1 * elementSize;
    case fx::gltf::Accessor::Type::Vec2:
      return 2 * elementSize;
    case fx::gltf::Accessor::Type::Vec3:
      return 3 * elementSize;
    case fx::gltf::Accessor::Type::Vec4:
      return 4 * elementSize;
    case fx::gltf::Accessor::Type::Mat2:
      return 4 * elementSize;
    case fx::gltf::Accessor::Type::Mat3:
      return 9 * elementSize;
    case fx::gltf::Accessor::Type::Mat4:
      return 16 * elementSize;
  }
  return 0; // Only to suppress compiler warning
}

auto LoadImage(
  const Context&  context,
  const Image&    image,
  vk::ArrayProxy<const std::filesystem::path> paths
) -> void
{
  char* hostPtr {nullptr};
  vmaMapMemory(context.allocator, image.allocation, reinterpret_cast<void**>(&hostPtr));
  auto layer = 0U;
  for (const auto& path : paths)
  {
    auto width    = 0;
    auto height   = 0;
    auto channels = 0;
    constexpr auto preferred = STBI_rgb_alpha; // TODO: Parameterize
    const auto data = stbi_load(path.c_str(), &width, &height, &channels, preferred);
    debug_print("Loading %dx%dx%d image %s", width, height, channels, path.filename().c_str());

    const auto layout = context.device.getImageSubresourceLayout(
      image.image, {image.aspect, 0, layer++});
    std::memcpy(hostPtr + layout.offset, data, width * height * preferred);

    stbi_image_free(data);
  }
  vmaUnmapMemory(context.allocator, image.allocation);
}

auto GenerateMipmaps(
  const Context&  context,
  const Image&    image,
  vk::ImageLayout finalLayout
) -> void
{
  vku::SubmitImmediate(context, [&](vk::CommandBuffer cmdBuf)
  {
    const auto numLayers = image.arrayLayers;
    const auto numLevels = image.mipLevels;

    vku::SetImageLayout(
      cmdBuf, image,
      vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferDstOptimal,
      {image.aspect, 0, numLevels, 0, numLayers});

    for (auto layer = 0U; layer < numLayers; layer++)
    {
      int mipWidth  = image.extent.width;
      int mipHeight = image.extent.height;

      for (auto level = 1U; level < numLevels; level++)
      {
        vk::ImageBlit region;
        region.srcSubresource = {image.aspect, level - 1, layer, 1};
        region.srcOffsets[0]  = vk::Offset3D {0, 0, 0};
        region.srcOffsets[1]  = vk::Offset3D {std::max(1, mipWidth), std::max(1, mipHeight), 1};
        mipWidth  >>= 1;
        mipHeight >>= 1;
        region.dstSubresource = {image.aspect, level - 0, layer, 1};
        region.dstOffsets[0]  = vk::Offset3D {0, 0, 0};
        region.dstOffsets[1]  = vk::Offset3D {std::max(1, mipWidth), std::max(1, mipHeight), 1};

        vku::SetImageLayout(
          cmdBuf, image,
          vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal,
          {image.aspect, level - 1, 1, layer, 1});

        cmdBuf.blitImage(
          image.image, vk::ImageLayout::eTransferSrcOptimal,
          image.image, vk::ImageLayout::eTransferDstOptimal,
          region, vk::Filter::eLinear);
      } // levels
    } // layers

    vku::SetImageLayout(
      cmdBuf, image,
      vk::ImageLayout::eTransferSrcOptimal, finalLayout,
      {image.aspect, 0, numLevels - 1, 0, numLayers});

    vku::SetImageLayout(
      cmdBuf, image,
      vk::ImageLayout::eTransferDstOptimal, finalLayout,
      {image.aspect, numLevels - 1, 1, 0, numLayers});
  });
}

auto LoadScene(const Context& context, const std::filesystem::path& path) -> Scene
{
  Scene scene;
  auto indexBufferOffset  = 0U;
  auto vertexBufferOffset = 0U;

  const auto gltfScene = fx::gltf::LoadFromText(path);
  debug_print("Loading glTF scene with %d meshe(s)", gltfScene.meshes.size());

  std::map<uint32_t, std::filesystem::path> uniqueImagePaths;

  for (const auto& gltfMesh : gltfScene.meshes)
  {
    debug_print("-- Loading mesh %s with %d primitive(s)",
      gltfMesh.name.c_str(), gltfMesh.primitives.size());

    for (const auto& primitive : gltfMesh.primitives)
    {
      // TODO: Verify that attributes are of expected data types
      // TODO: Support more attribute types
      const auto indexBuffer    = vku::GetGLTFBuffer<uint16_t, 1>(gltfScene, primitive.indices);
      const auto positionBuffer = vku::GetGLTFBuffer<float,    3>(gltfScene, primitive.attributes.at("POSITION"));
      const auto normalBuffer   = vku::GetGLTFBuffer<float,    3>(gltfScene, primitive.attributes.at("NORMAL"));
      const auto texcoordBuffer = vku::GetGLTFBuffer<float,    2>(gltfScene, primitive.attributes.at("TEXCOORD_0"));

      scene.indices.insert(std::end(scene.indices), indexBuffer.data, indexBuffer.data + indexBuffer.count);

      // Interleave attributes into single vertex buffer
      for (auto i = 0; i < positionBuffer.count; i++)
      {
        vku::Scene::Vertex vertex
        {
          positionBuffer.data[positionBuffer.stride * i + 0],
          positionBuffer.data[positionBuffer.stride * i + 1],
          positionBuffer.data[positionBuffer.stride * i + 2],
          normalBuffer.data  [normalBuffer.stride   * i + 0],
          normalBuffer.data  [normalBuffer.stride   * i + 1],
          normalBuffer.data  [normalBuffer.stride   * i + 2],
          texcoordBuffer.data[texcoordBuffer.stride * i + 0],
          texcoordBuffer.data[texcoordBuffer.stride * i + 1],
        };
        scene.vertices.push_back(vertex);
      }

      // TODO: First check if material is present
      const auto& material  = gltfScene.materials[primitive.material];
      const auto  textureId = material.pbrMetallicRoughness.baseColorTexture.index;
      const auto  imageId   = gltfScene.textures[textureId].source;

      const auto& filename  = gltfScene.images[imageId].uri;
      const auto  imagePath = path.parent_path() / filename;

      const auto [iter, status] = uniqueImagePaths.emplace(imageId, imagePath);
      const uint32_t newImageId = std::distance(std::begin(uniqueImagePaths), iter);

      scene.drawInfos.push_back({indexBuffer.count, indexBufferOffset, vertexBufferOffset, newImageId});
      indexBufferOffset  += indexBuffer.count;
      vertexBufferOffset += positionBuffer.count;

      debug_print("---- Loaded primitive with %6d vertices and image ID %d",
        positionBuffer.count, newImageId);
   }
  }

  std::vector<std::filesystem::path> imagePaths (uniqueImagePaths.size());
  std::transform(
    std::cbegin(uniqueImagePaths), std::cend(uniqueImagePaths),
    std::begin(imagePaths),
    [](const auto& p) -> std::filesystem::path { return p.second; });

  // TODO: Use per-mesh image array
  debug_print("-- Loading %d images in scene", imagePaths.size());
  constexpr vk::Extent3D extent {1024, 1024, 1}; // TODO: Parameterize
  scene.imageArray = vku::ImageBuilder(
    extent, vk::Format::eR8G8B8A8Srgb, imagePaths)
    .SetUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
    .SetMipLevels(vku::ImageBuilder::sAllMipLevels)
    .Build(context);

  return scene;
}

auto DrawScene(
  vk::CommandBuffer cmdBuf,
  vk::PipelineLayout pipelineLayout,
  const Scene& scene
) -> void
{
  for (const auto& di : scene.drawInfos)
  {
    vku::PushConstant<DrawPushConstant> pushConstant {di.imageId};
    vku::BindPushConstant(cmdBuf, pipelineLayout, pushConstant);
    cmdBuf.drawIndexed(di.indexCount, 1, di.indexOffset, di.vertexOffset, 0);
  }
}

auto CalculateMipLevels(vk::Extent2D extent) -> uint32_t
{
  return 1 + static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height))));
}

} // namespace vulkan_utils

Renderer::Renderer(
  int width,
  int height,
  const std::filesystem::path& assetPath)
  : m_extent {static_cast<unsigned>(width), static_cast<unsigned>(height), 1}
  , m_frame  {width, height, 4, std::vector<char>(width * height * 4)}
{
  if (const auto handle = dlopen("/usr/lib64/librenderdoc.so", RTLD_NOW | RTLD_NOLOAD))
  {
    const auto RENDERDOC_GetAPI = reinterpret_cast<pRENDERDOC_GetAPI>(
      dlsym(handle, "RENDERDOC_GetAPI"));
    const auto status = RENDERDOC_GetAPI(
      eRENDERDOC_API_Version_1_4_2, reinterpret_cast<void**>(&m_renderdoc));
    ASSERT_THROW(status == 1, "Failed to setup renderdoc app API");
    debug_print("Using renderdoc app API");
  }

  if (m_renderdoc) m_renderdoc->StartFrameCapture(nullptr, nullptr);

  m_context = vku::CreateContext();

  m_scene = vku::LoadScene(m_context, assetPath);

  /////////////////////////////////////////////////////////////////////////////
  // Camera matrices
  /////////////////////////////////////////////////////////////////////////////

  const auto aspectRatio   = static_cast<double>(m_extent.width) / m_extent.height;
  const glm::mat4 modelMat = glm::scale(glm::mat4 {1}, glm::vec3 {0.008});
  const glm::mat4 viewMat  = glm::translate(
    glm::rotate(glm::mat4 {1}, glm::radians(90.0F), glm::vec3 {0, -1, 0}),
    glm::vec3 {0, -1, 0});
  const glm::mat4 projMat  = glm::perspective(glm::radians(60.0), aspectRatio, 0.1, 100.0);

  m_frameData.mvpMat    = projMat * viewMat * modelMat;
  m_frameData.modelMat  = modelMat;
  m_frameData.normalMat = glm::transpose(glm::inverse(modelMat));

  /////////////////////////////////////////////////////////////////////////////
  // Buffers
  /////////////////////////////////////////////////////////////////////////////

  m_uniformBuffer = vku::CreateBuffer(
    m_context,
    sizeof(m_frameData),
    vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
    VMA_MEMORY_USAGE_CPU_TO_GPU);

  vku::CopyBuffer(m_context, std::array {m_frameData}, m_uniformBuffer);

  m_vertexBuffer = vku::CreateBuffer(
    m_context,
    vku::SizeInBytes(m_scene.vertices),
    vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
    VMA_MEMORY_USAGE_CPU_TO_GPU);

  vku::CopyBuffer(m_context, m_scene.vertices, m_vertexBuffer);

  m_indexBuffer = vku::CreateBuffer(
    m_context,
    vku::SizeInBytes(m_scene.indices),
    vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
    VMA_MEMORY_USAGE_CPU_TO_GPU);

  vku::CopyBuffer(m_context, m_scene.indices, m_indexBuffer);

  /////////////////////////////////////////////////////////////////////////////
  // Images
  /////////////////////////////////////////////////////////////////////////////

  m_colorImage = vku::ImageBuilder(
    m_extent, vk::Format::eR8G8B8A8Srgb, VMA_MEMORY_USAGE_GPU_ONLY)
    .SetUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc)
    .Build(m_context);

  m_depthImage = vku::ImageBuilder(
    m_extent, vk::Format::eD32Sfloat, VMA_MEMORY_USAGE_GPU_ONLY)
    .SetUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
    .Build(m_context);

  m_copyImage = vku::ImageBuilder(
    m_extent, vk::Format::eR8G8B8A8Unorm, VMA_MEMORY_USAGE_GPU_TO_CPU)
    .SetUsage(vk::ImageUsageFlagBits::eTransferDst)
    .Build(m_context);

  /////////////////////////////////////////////////////////////////////////////
  // Render Pass
  /////////////////////////////////////////////////////////////////////////////

  const std::array<vk::AttachmentDescription, 2> attachments
  {
    vku::CreateAttachmentDescription(m_colorImage, vk::ImageLayout::eTransferSrcOptimal),
    vku::CreateAttachmentDescription(m_depthImage),
  };

  const vk::AttachmentReference colorAttachmentRef {0, vk::ImageLayout::eColorAttachmentOptimal};
  const vk::AttachmentReference depthAttachmentRef {1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

  std::array<vk::SubpassDescription, 1> subpasses;
  subpasses[0].pipelineBindPoint        = vk::PipelineBindPoint::eGraphics;
  subpasses[0].colorAttachmentCount     = 1;
  subpasses[0].pColorAttachments        = &colorAttachmentRef;
  subpasses[0].pDepthStencilAttachment  = &depthAttachmentRef;

  /*
  // TODO: Add subpass dependency
  std::array<vk::SubpassDependency, 2> dependencies;
  dependencies[0] = {
    VK_SUBPASS_EXTERNAL,
    0,
    vk::PipelineStageFlags::eBottomOfPipe,
    vk::PipelineStageFlags::eColorAttachmentOutput,
    vk::AccessFlagBits::eMemoryRead,
    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
    vk::DependencyFlags::eByRegion,
  };
  */

  vk::RenderPassCreateInfo renderPassCI;
  renderPassCI.attachmentCount = attachments.size();
  renderPassCI.pAttachments    = attachments.data();
  renderPassCI.subpassCount    = subpasses.size();
  renderPassCI.pSubpasses      = subpasses.data();
  m_renderPass = m_context.device.createRenderPass(renderPassCI);

  /////////////////////////////////////////////////////////////////////////////
  // Framebuffer
  /////////////////////////////////////////////////////////////////////////////

  const std::array<vk::ImageView, 2> imageViews
  {
    m_colorImage.views[0],
    m_depthImage.views[0],
  };

  vk::FramebufferCreateInfo framebufferCI;
  framebufferCI.renderPass      = m_renderPass;
  framebufferCI.attachmentCount = imageViews.size();
  framebufferCI.pAttachments    = imageViews.data();
  framebufferCI.width           = m_extent.width;
  framebufferCI.height          = m_extent.height;
  framebufferCI.layers          = 1;
  m_framebuffer = m_context.device.createFramebuffer(framebufferCI);

  /////////////////////////////////////////////////////////////////////////////
  // Descriptor Sets
  /////////////////////////////////////////////////////////////////////////////

  m_descSet = vku::CreateDescriptorSet(
    m_context,
    {
      {vk::DescriptorType::eUniformBuffer},
      {vk::DescriptorType::eStorageBuffer},
      {vk::DescriptorType::eCombinedImageSampler, static_cast<uint32_t>(m_scene.imageArray.views.size())},
    });

  vku::UpdateDescriptorSet(m_context, m_descSet, 0, m_uniformBuffer);
  vku::UpdateDescriptorSet(m_context, m_descSet, 1, m_vertexBuffer);
  vku::UpdateDescriptorSet(m_context, m_descSet, 2, m_scene.imageArray);

  /////////////////////////////////////////////////////////////////////////////
  // Graphics Pipeline
  /////////////////////////////////////////////////////////////////////////////

  m_pipelineLayout = vku::CreatePipelineLayout(
    m_context, m_descSet, vku::PushConstant<DrawPushConstant> {});

  const auto shaderStages = vku::CreateGraphicsPipelineShaderStages(
    m_context, "shaders/default.vert.spv", "shaders/default.frag.spv");

  vk::PipelineVertexInputStateCreateInfo vertexInputState; // Empty for manual vertex attribute fetch in shader

  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
  inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;

  vk::PipelineRasterizationStateCreateInfo rasterizationState;
  rasterizationState.polygonMode  = vk::PolygonMode::eFill;
  rasterizationState.cullMode     = vk::CullModeFlagBits::eBack;
  rasterizationState.frontFace    = vk::FrontFace::eCounterClockwise;
  rasterizationState.lineWidth    = 1;

  vk::Viewport viewport;
  viewport.x        = 0;
  viewport.y        = m_extent.height;
  viewport.width    = m_extent.width;
  viewport.height   = -static_cast<float>(m_extent.height);
  viewport.minDepth = 0;
  viewport.maxDepth = 1;

  vk::Rect2D scissor;
  scissor.offset = vk::Offset2D {0, 0};
  scissor.extent = vk::Extent2D {m_extent.width, m_extent.height};

  vk::PipelineViewportStateCreateInfo viewportState;
  viewportState.viewportCount = 1;
  viewportState.pViewports    = &viewport;
  viewportState.scissorCount  = 1;
  viewportState.pScissors     = &scissor;

  vk::PipelineMultisampleStateCreateInfo multisampleState;
  multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;

  vk::PipelineDepthStencilStateCreateInfo depthStencilState;
  depthStencilState.depthTestEnable  = true;
  depthStencilState.depthWriteEnable = true;
  depthStencilState.depthCompareOp   = vk::CompareOp::eLessOrEqual;

  vk::PipelineColorBlendAttachmentState blendAttachmentState;
  blendAttachmentState.colorWriteMask =
    vk::ColorComponentFlagBits::eR |
    vk::ColorComponentFlagBits::eG |
    vk::ColorComponentFlagBits::eB |
    vk::ColorComponentFlagBits::eA;

  vk::PipelineColorBlendStateCreateInfo colorBlendState;
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments    = &blendAttachmentState;

  vk::GraphicsPipelineCreateInfo pipelineCI;
  pipelineCI.stageCount           = static_cast<uint32_t>(shaderStages.size());
  pipelineCI.pStages              = shaderStages.data();
  pipelineCI.pVertexInputState    = &vertexInputState;
  pipelineCI.pInputAssemblyState  = &inputAssemblyState;
  pipelineCI.pViewportState       = &viewportState;
  pipelineCI.pRasterizationState  = &rasterizationState;
  pipelineCI.pMultisampleState    = &multisampleState;
  pipelineCI.pDepthStencilState   = &depthStencilState;
  pipelineCI.pColorBlendState     = &colorBlendState;
  pipelineCI.layout               = m_pipelineLayout;
  pipelineCI.renderPass           = m_renderPass;
  const auto pipelineStatus = m_context.device.createGraphicsPipeline(nullptr, pipelineCI);
  ASSERT_THROW(pipelineStatus.result == vk::Result::eSuccess,
    "Failed to create vulkan graphics pipeline!");
  m_pipeline = pipelineStatus.value;

  debug_print("Initialized vulkan");
}

Renderer::~Renderer()
{
  if (m_renderdoc) m_renderdoc->EndFrameCapture(nullptr, nullptr);
}

auto Renderer::Render() -> void
{
  /////////////////////////////////////////////////////////////////////////////
  // Render
  /////////////////////////////////////////////////////////////////////////////

  std::array<vk::ClearValue, 2> clearValues;
  clearValues[0].color = std::array<float, 4> {{0.25, 0.5, 1, 1}};
  clearValues[1].depthStencil = {{1.0F, 1}};

  vk::RenderPassBeginInfo renderPassBI;
  renderPassBI.renderPass      = m_renderPass;
  renderPassBI.framebuffer     = m_framebuffer;
  renderPassBI.renderArea      = vk::Rect2D {{0, 0}, {m_extent.width, m_extent.height}};
  renderPassBI.clearValueCount = clearValues.size();
  renderPassBI.pClearValues    = clearValues.data();

  vku::SubmitImmediate(m_context, [&](vk::CommandBuffer cmdBuf)
  {
    cmdBuf.beginRenderPass(renderPassBI, vk::SubpassContents::eInline);
    cmdBuf.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
    cmdBuf.bindIndexBuffer(m_indexBuffer.buffer, 0, vk::IndexType::eUint16);
    vku::BindDescriptorSet(cmdBuf, m_pipelineLayout, m_descSet);
    vku::DrawScene(cmdBuf, m_pipelineLayout, m_scene);
    cmdBuf.endRenderPass();
  });

  /////////////////////////////////////////////////////////////////////////////
  // Readback
  /////////////////////////////////////////////////////////////////////////////

  const vk::ImageSubresourceRange subresourceRange {m_copyImage.aspect, 0, 1, 0, 1};

  std::array<vk::ImageCopy, 1> regions;
  regions[0].srcSubresource = {m_colorImage.aspect, 0, 0, 1};
  regions[0].dstSubresource = {m_copyImage.aspect,  0, 0, 1};
  regions[0].extent = m_extent;

  vku::SubmitImmediate(m_context, [&](vk::CommandBuffer cmdBuf)
  {
    vku::SetImageLayout(
      cmdBuf, m_copyImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, subresourceRange);

    cmdBuf.copyImage(
      m_colorImage.image, vk::ImageLayout::eTransferSrcOptimal,
      m_copyImage.image,  vk::ImageLayout::eTransferDstOptimal,
      regions);

    vku::SetImageLayout(
      cmdBuf, m_copyImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eGeneral, subresourceRange);
  });

  vku::CopyBuffer(m_context, m_copyImage, m_frame.data);
}
