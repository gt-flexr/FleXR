#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

#define VMA_STATIC_VULKAN_FUNCTIONS   0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS  0
#include <vk_mem_alloc.h>

#include <renderdoc_app.h>

struct Image
{
  vk::Image     image;
  vk::ImageView view;
  VmaAllocation allocation;
};

class Renderer
{
public:
  Renderer(int width, int height);

  ~Renderer();

  auto Tick() -> void;

  auto SubmitWork(vk::CommandBuffer commandBuffer) -> void;

private:
  RENDERDOC_API_1_4_2* m_renderdoc {nullptr};

  VmaAllocator        m_allocator;

  vk::Extent3D        m_extent;
  vk::DynamicLoader   m_dl;
  vk::Instance        m_instance;
  vk::PhysicalDevice  m_physicalDevice;
  vk::Device          m_device;
  vk::Queue           m_queue;
  vk::CommandPool     m_commandPool;
  vk::RenderPass      m_renderPass;
  vk::Framebuffer     m_framebuffer;

  Image               m_framebufferImage;
  Image               m_copyImage;
};
