#include "kernels/source/renderer.h"

#include <array>
#include <algorithm>
#include <dlfcn.h> // For dlopen
#include <fstream>
#include <vector>

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
    .add_required_extension("VK_KHR_maintenance1")
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

ImageBuilder::ImageBuilder(vk::Extent3D extent, vk::Format format, VmaMemoryUsage usage)
  : usage {usage}
{
  imageCI.imageType     = vk::ImageType::e2D;
  imageCI.format        = format;
  imageCI.extent        = extent;
  imageCI.arrayLayers   = 1;
  imageCI.mipLevels     = 1;
  imageCI.initialLayout = vk::ImageLayout::eUndefined;
  imageCI.samples       = vk::SampleCountFlagBits::e1;
  imageCI.tiling        = vk::ImageTiling::eOptimal;
  imageCI.usage         = vk::ImageUsageFlagBits::eColorAttachment;
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

auto ImageBuilder::Build(const Context& context) const -> Image
{
  VmaAllocationCreateInfo allocationCI {};
  allocationCI.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;

  VkImage vkImage;
  VmaAllocation allocation {};
  const auto imageStatus = vmaCreateImage(
    context.allocator,
    reinterpret_cast<const VkImageCreateInfo*>(&imageCI),
    &allocationCI,
    &vkImage,
    &allocation,
    nullptr);
  ASSERT_THROW(imageStatus == VK_SUCCESS, "Failed to create vulkan image! %d", imageStatus);
  const auto image = vk::Image {vkImage};

  constexpr auto validImageViewUsageFlags =
    vk::ImageUsageFlagBits::eSampled                |
    vk::ImageUsageFlagBits::eStorage                |
    vk::ImageUsageFlagBits::eColorAttachment        |
    vk::ImageUsageFlagBits::eDepthStencilAttachment |
    vk::ImageUsageFlagBits::eInputAttachment        |
    vk::ImageUsageFlagBits::eTransientAttachment;

  if (imageCI.usage & validImageViewUsageFlags)
  {
    const auto aspect = imageCI.usage == vk::ImageUsageFlagBits::eDepthStencilAttachment ?
      vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;

    vk::ImageViewCreateInfo imageViewCI;
    imageViewCI.image    = image;
    imageViewCI.viewType = vk::ImageViewType::e2D;
    imageViewCI.format   = imageCI.format; // TODO: Support aliased formats
    imageViewCI.subresourceRange = {aspect, 0, 1, 0, 1};
    const auto view = context.device.createImageView(imageViewCI);
    return {allocation, imageCI.format, imageCI.usage, imageCI.initialLayout, image, view};
  }

  return {allocation, imageCI.format, imageCI.usage, imageCI.initialLayout, image, std::nullopt};
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

auto CreateShaderModule(
  const Context& context,
  const std::filesystem::path& path) -> vk::ShaderModule
{
  std::basic_ifstream<char> file {path, std::ios::binary};
  ASSERT_THROW(file.is_open(), "Failed to open SPIRV shader! %s", path.c_str());
  const std::vector<char> data {std::istreambuf_iterator<char> {file}, {}};

  vk::ShaderModuleCreateInfo shaderModuleCI;
  shaderModuleCI.codeSize = data.size();
  shaderModuleCI.pCode    = reinterpret_cast<const uint32_t*>(data.data());
  return context.device.createShaderModule(shaderModuleCI);
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
  desc.initialLayout = image.initialLayout;
  desc.finalLayout   = isDepthAttachment ? vk::ImageLayout::eDepthStencilAttachmentOptimal : finalLayout;
  return desc;
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

  struct Vertex { float x {0}, y {0}, z {0}; };
  constexpr std::array<Vertex, 6> vertices
  {{
    {-1, -1, 1}, {+1, -1, 1}, {+1, +1, 1},
    {+1, +1, 1}, {-1, +1, 1}, {-1, -1, 1},
  }};

  /////////////////////////////////////////////////////////////////////////////
  // Buffers
  /////////////////////////////////////////////////////////////////////////////

  m_vertexStorageBuffer = vku::CreateBuffer(
    m_context,
    vku::SizeInBytes(vertices),
    vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
    VMA_MEMORY_USAGE_CPU_TO_GPU);

  vku::CopyBuffer(m_context, vertices, m_vertexStorageBuffer);

  /////////////////////////////////////////////////////////////////////////////
  // Images
  /////////////////////////////////////////////////////////////////////////////

  m_colorImage = vku::ImageBuilder(
    m_extent, vk::Format::eR8G8B8A8Unorm, VMA_MEMORY_USAGE_GPU_ONLY)
    .SetUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc)
    .Build(m_context);

  m_depthImage = vku::ImageBuilder(
    m_extent, vk::Format::eD32Sfloat, VMA_MEMORY_USAGE_GPU_ONLY)
    .SetUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
    .Build(m_context);

  m_copyImage = vku::ImageBuilder(
    m_extent, vk::Format::eR8G8B8A8Unorm, VMA_MEMORY_USAGE_GPU_TO_CPU)
    .SetUsage(vk::ImageUsageFlagBits::eTransferDst)
    .SetTiling(vk::ImageTiling::eLinear)
    .Build(m_context);

  /////////////////////////////////////////////////////////////////////////////
  // Render Pass
  /////////////////////////////////////////////////////////////////////////////

  const std::array<vk::AttachmentDescription, 2> attachments
  {
    CreateAttachmentDescription(m_colorImage, vk::ImageLayout::eTransferSrcOptimal),
    CreateAttachmentDescription(m_depthImage),
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
    m_colorImage.view.value(),
    m_depthImage.view.value(),
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

  std::array<vk::DescriptorSetLayoutBinding, 1> bindings;
  bindings[0].binding         = 0;
  bindings[0].descriptorType  = vk::DescriptorType::eStorageBuffer;
  bindings[0].descriptorCount = 1;
  bindings[0].stageFlags      = vk::ShaderStageFlagBits::eAllGraphics;

  vk::DescriptorSetLayoutCreateInfo descSetLayoutCI;
  descSetLayoutCI.bindingCount  = bindings.size();
  descSetLayoutCI.pBindings     = bindings.data();
  const auto descSetLayout = m_context.device.createDescriptorSetLayout(descSetLayoutCI);

  std::array<vk::DescriptorPoolSize, 1> descPoolSize;
  descPoolSize[0].type            = vk::DescriptorType::eStorageBuffer;
  descPoolSize[0].descriptorCount = 1;

  vk::DescriptorPoolCreateInfo descPoolCI;
  descPoolCI.maxSets        = 1;
  descPoolCI.poolSizeCount  = descPoolSize.size();
  descPoolCI.pPoolSizes     = descPoolSize.data();
  const auto descPool = m_context.device.createDescriptorPool(descPoolCI);

  vk::DescriptorSetAllocateInfo descSetAI;
  descSetAI.descriptorPool      = descPool;
  descSetAI.descriptorSetCount  = 1;
  descSetAI.pSetLayouts         = &descSetLayout;
  m_descSet = m_context.device.allocateDescriptorSets(descSetAI)[0];

  vk::DescriptorBufferInfo descBI;
  descBI.buffer = m_vertexStorageBuffer.buffer;
  descBI.offset = 0;
  descBI.range  = VK_WHOLE_SIZE;

  vk::WriteDescriptorSet writeDescSet;
  writeDescSet.dstSet           = m_descSet;
  writeDescSet.dstBinding       = 0;
  writeDescSet.descriptorCount  = 1;
  writeDescSet.descriptorType   = vk::DescriptorType::eStorageBuffer;
  writeDescSet.pBufferInfo      = &descBI;
  m_context.device.updateDescriptorSets(writeDescSet, {});

  /////////////////////////////////////////////////////////////////////////////
  // Graphics Pipeline
  /////////////////////////////////////////////////////////////////////////////

  vk::PipelineLayoutCreateInfo pipelineLayoutCI;
  pipelineLayoutCI.setLayoutCount = 1;
  pipelineLayoutCI.pSetLayouts    = &descSetLayout;
  m_pipelineLayout = m_context.device.createPipelineLayout(pipelineLayoutCI);

  std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;
  shaderStages[0].stage  = vk::ShaderStageFlagBits::eVertex;
  shaderStages[0].pName  = "main";
  shaderStages[0].module = vku::CreateShaderModule(m_context, "default.vert.spv");
  shaderStages[1].stage  = vk::ShaderStageFlagBits::eFragment;
  shaderStages[1].pName  = "main";
  shaderStages[1].module = vku::CreateShaderModule(m_context, "default.frag.spv");

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
  depthStencilState.depthCompareOp   = vk::CompareOp::eAlways;

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

  const auto commandBuffers = m_context.device.allocateCommandBuffers({
    m_commandPool, vk::CommandBufferLevel::ePrimary, 2});

  std::array<vk::ClearValue, 2> clearValues;
  clearValues[0].color = std::array<float, 4> {{0.25, 0.5, 1, 1}};
  clearValues[1].depthStencil = {{1.0F, 1}};

  vk::RenderPassBeginInfo renderPassBI;
  renderPassBI.renderPass      = m_renderPass;
  renderPassBI.framebuffer     = m_framebuffer;
  renderPassBI.renderArea      = vk::Rect2D {{0, 0}, {m_extent.width, m_extent.height}};
  renderPassBI.clearValueCount = clearValues.size();
  renderPassBI.pClearValues    = clearValues.data();

  commandBuffers[0].begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  commandBuffers[0].beginRenderPass(renderPassBI, vk::SubpassContents::eInline);
  commandBuffers[0].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
  commandBuffers[0].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0, m_descSet, {});
  commandBuffers[0].draw(6, 1, 0, 0);
  commandBuffers[0].endRenderPass();
  commandBuffers[0].end();

  vku::SubmitWork(m_context, commandBuffers[0]);

  /////////////////////////////////////////////////////////////////////////////
  // Readback
  /////////////////////////////////////////////////////////////////////////////

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
    m_colorImage.image, vk::ImageLayout::eTransferSrcOptimal,
    m_copyImage.image,  vk::ImageLayout::eTransferDstOptimal,
    regions);
  commandBuffers[1].pipelineBarrier(
    vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barriers[1]);
  commandBuffers[1].end();

  vku::SubmitWork(m_context, commandBuffers[1]);

  vku::CopyBuffer(m_context, m_copyImage, m_frame.data);
}
