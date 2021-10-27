#include "kernels/source/renderer.h"

#include <array>
#include <algorithm>
#include <dlfcn.h> // For dlopen

#include <VkBootstrap.h>

#define VMA_ASSERT(expr) assert(expr)
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#define ASSERT_THROW(status, ...) \
if (!(status)) { \
  debug_print(__VA_ARGS__); \
  throw; \
}

#ifndef debug_print
#define debug_print(...) do { \
  fprintf(stderr, "\033[1;31m[DEBUG] \033[0;32m[FUNC] %s \033[0m", __PRETTY_FUNCTION__); \
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

  vkb::PhysicalDeviceSelector physicalDeviceSelector {vkbInstance};
  const auto physicalDeviceStatus = physicalDeviceSelector
    .set_minimum_version(1, 2)
    .add_required_extension("VK_KHR_get_memory_requirements2")
    .add_required_extension("VK_KHR_bind_memory2")
    .select();
  ASSERT_THROW(physicalDeviceStatus,
    "Failed to select vulkan physical device! %s", physicalDeviceStatus.error().message().c_str());
  const auto vkbPhysicalDevice = physicalDeviceStatus.value();
  const auto physicalDevice = vk::PhysicalDevice {vkbPhysicalDevice.physical_device};

  const auto physicalDeviceProps = physicalDevice.getProperties();
  const std::string deviceName = physicalDeviceProps.deviceName;
  debug_print(
    "\nVulkan physical device:\n"
    "- name:   %s\n"
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

  const auto queueStatus = vkbDevice.get_queue(vkb::QueueType::graphics);
  ASSERT_THROW(queueStatus,
    "Failed to get vulkan queue! %s", queueStatus.error().message().c_str());
  const auto queue = vk::Queue {queueStatus.value()};

  return {instance, physicalDevice, device, queue, allocator};
}

} // namespace vulkan_utils

Renderer::Renderer(int width, int height)
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

//   const auto queueFamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
  const auto queueFamilyIndex = 0; // TODO: Get queue index from context
  m_commandPool = m_context.device.createCommandPool({{}, queueFamilyIndex});

  // Framebuffer image
  {
  vk::ImageCreateInfo imageCI;
  imageCI.imageType     = vk::ImageType::e2D;
  imageCI.format        = vk::Format::eR8G8B8A8Unorm;
  imageCI.extent        = m_extent;
  imageCI.arrayLayers   = 1;
  imageCI.mipLevels     = 1;
  imageCI.initialLayout = vk::ImageLayout::eUndefined;
  imageCI.samples       = vk::SampleCountFlagBits::e1;
  imageCI.tiling        = vk::ImageTiling::eOptimal;
  imageCI.usage         = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc;

  VmaAllocationCreateInfo allocationCI {};
  allocationCI.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  VkImage image;
  const auto imageStatus = vmaCreateImage(
    m_context.allocator,
    reinterpret_cast<const VkImageCreateInfo*>(&imageCI),
    &allocationCI,
    &image,
    &m_framebufferImage.allocation,
    nullptr);
  ASSERT_THROW(imageStatus == VK_SUCCESS, "Failed to create vulkan image! %d", imageStatus);
  m_framebufferImage.image = vk::Image {image};

  vk::ImageViewCreateInfo imageViewCI;
  imageViewCI.image    = m_framebufferImage.image;
  imageViewCI.viewType = vk::ImageViewType::e2D;
  imageViewCI.format   = vk::Format::eR8G8B8A8Unorm;
  imageViewCI.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
  m_framebufferImage.view = m_context.device.createImageView(imageViewCI);
  }

  // Read back
  {
  vk::ImageCreateInfo imageCI;
  imageCI.imageType     = vk::ImageType::e2D;
  imageCI.format        = vk::Format::eR8G8B8A8Unorm;
  imageCI.extent        = m_extent;
  imageCI.arrayLayers   = 1;
  imageCI.mipLevels     = 1;
  imageCI.initialLayout = vk::ImageLayout::eUndefined;
  imageCI.samples       = vk::SampleCountFlagBits::e1;
  imageCI.tiling        = vk::ImageTiling::eLinear;
  imageCI.usage         = vk::ImageUsageFlagBits::eTransferDst;

  VmaAllocationCreateInfo allocationCI {};
  allocationCI.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;

  VkImage image;
  const auto imageStatus = vmaCreateImage(
    m_context.allocator,
    reinterpret_cast<const VkImageCreateInfo*>(&imageCI),
    &allocationCI,
    &image,
    &m_copyImage.allocation,
    nullptr);
  ASSERT_THROW(imageStatus == VK_SUCCESS, "Failed to create vulkan image! %d", imageStatus);
  m_copyImage.image = vk::Image {image};
  }

  // Renderpass
  std::array<vk::AttachmentDescription, 1> attachments;
  attachments[0].format        = vk::Format::eR8G8B8A8Unorm;
  attachments[0].samples       = vk::SampleCountFlagBits::e1;
  attachments[0].loadOp        = vk::AttachmentLoadOp::eClear;
  attachments[0].storeOp       = vk::AttachmentStoreOp::eStore;
  attachments[0].initialLayout = vk::ImageLayout::eUndefined;
  attachments[0].finalLayout   = vk::ImageLayout::eTransferSrcOptimal;

  const vk::AttachmentReference colorAttachmentRef {0, vk::ImageLayout::eColorAttachmentOptimal};

  std::array<vk::SubpassDescription, 1> subpasses;
  subpasses[0].pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
  subpasses[0].colorAttachmentCount = 1;
  subpasses[0].pColorAttachments    = &colorAttachmentRef;

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

  std::array<vk::ImageView, 1> imageViews;
  imageViews[0] = m_framebufferImage.view;

  vk::FramebufferCreateInfo framebufferCI;
  framebufferCI.renderPass      = m_renderPass;
  framebufferCI.attachmentCount = imageViews.size();
  framebufferCI.pAttachments    = imageViews.data();
  framebufferCI.width           = m_extent.width;
  framebufferCI.height          = m_extent.height;
  framebufferCI.layers          = 1;;
  m_framebuffer = m_context.device.createFramebuffer(framebufferCI);

  debug_print("Initialized vulkan");
}

Renderer::~Renderer()
{
  if (m_renderdoc) m_renderdoc->EndFrameCapture(nullptr, nullptr);
}

auto Renderer::SubmitWork(vk::CommandBuffer commandBuffer) -> void
{
  vk::SubmitInfo submitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer;
  const auto fence = m_context.device.createFence({});
  m_context.queue.submit(submitInfo, fence);
  const auto status = m_context.device.waitForFences(fence, true, std::numeric_limits<uint64_t>::max());
  ASSERT_THROW(status == vk::Result::eSuccess, "Failed to wait for vulkan fence!");
  m_context.device.destroyFence(fence);
}

auto Renderer::Render() -> void
{
  const auto commandBuffers = m_context.device.allocateCommandBuffers({
    m_commandPool, vk::CommandBufferLevel::ePrimary, 2});

  std::array<vk::ClearValue, 1> clearValues;
  clearValues[0].color = std::array<float, 4> {{1, 0, 1, 1}};

  vk::RenderPassBeginInfo renderPassBI;
  renderPassBI.renderPass      = m_renderPass;
  renderPassBI.framebuffer     = m_framebuffer;
  renderPassBI.renderArea      = vk::Rect2D {{0, 0}, {m_extent.width, m_extent.height}};
  renderPassBI.clearValueCount = clearValues.size();
  renderPassBI.pClearValues    = clearValues.data();

  std::array<vk::ClearAttachment, 1> clearAttachments;
  clearAttachments[0].aspectMask = vk::ImageAspectFlagBits::eColor;
  clearAttachments[0].colorAttachment = 0;
  clearAttachments[0].clearValue = vk::ClearColorValue {std::array<float, 4> {{0.25, 0.5, 1, 1}}};

  std::array<vk::ClearRect, 1> clearRects;
  clearRects[0] = vk::ClearRect {{{0, 0}, {m_extent.width, m_extent.height}}, 0, 1};

  commandBuffers[0].begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  commandBuffers[0].beginRenderPass(renderPassBI, vk::SubpassContents::eInline);
  commandBuffers[0].clearAttachments(clearAttachments, clearRects);
  commandBuffers[0].endRenderPass();
  commandBuffers[0].end();

  SubmitWork(commandBuffers[0]);
  m_context.device.waitIdle();

  std::array<vk::ImageCopy, 1> regions;
  regions[0].srcSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
  regions[0].dstSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
  regions[0].extent = m_extent;

  std::array<vk::ImageMemoryBarrier, 2> barriers;

  barriers[0].srcAccessMask    = {};
  barriers[0].dstAccessMask    = vk::AccessFlagBits::eTransferWrite;
  barriers[0].oldLayout        = vk::ImageLayout::eUndefined;
  barriers[0].newLayout        = vk::ImageLayout::eTransferDstOptimal;
  barriers[0].image            = m_copyImage.image;
  barriers[0].subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

  barriers[1].srcAccessMask    = vk::AccessFlagBits::eTransferWrite;
  barriers[1].dstAccessMask    = vk::AccessFlagBits::eMemoryRead;
  barriers[1].oldLayout        = vk::ImageLayout::eTransferDstOptimal;
  barriers[1].newLayout        = vk::ImageLayout::eGeneral;
  barriers[1].image            = m_copyImage.image;
  barriers[1].subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

  commandBuffers[1].begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  commandBuffers[1].pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barriers[0]);
  commandBuffers[1].copyImage(
    m_framebufferImage.image, vk::ImageLayout::eTransferSrcOptimal,
    m_copyImage.image,        vk::ImageLayout::eTransferDstOptimal,
    regions);
  commandBuffers[1].pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barriers[1]);
  commandBuffers[1].end();

  SubmitWork(commandBuffers[1]);
  m_context.device.waitIdle();

  char* data {nullptr};
  vmaMapMemory(m_context.allocator, m_copyImage.allocation, reinterpret_cast<void**>(&data));
  std::memcpy(m_frame.data.data(), data, m_frame.data.size());
  vmaUnmapMemory(m_context.allocator, m_copyImage.allocation);
}
