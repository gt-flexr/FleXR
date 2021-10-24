include(FetchContent)

FetchContent_Declare(
  fetch_vk_bootstrap
  GIT_REPOSITORY  https://github.com/charles-lunarg/vk-bootstrap.git
  GIT_TAG         v0.4
  GIT_SHALLOW     ON
  GIT_PROGRESS    ON
)

FetchContent_Declare(
  fetch_vma
  GIT_REPOSITORY  https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG         v2.3.0
  GIT_SHALLOW     ON
  GIT_PROGRESS    ON
)

# TODO: Try to use GitHub links instead of raw links
# WARNING: Requires latest cmake 3.21+ otherwise SOURCE_DIR is not populated
FetchContent_Declare(
  fetch_renderdoc_app
  URL https://raw.githubusercontent.com/baldurk/renderdoc/v1.x/renderdoc/api/app/renderdoc_app.h
  DOWNLOAD_NO_EXTRACT ON
)

# TODO: Try to use GitHub links instead of raw links
# WARNING: Requires latest cmake 3.21+ otherwise SOURCE_DIR is not populated
FetchContent_Declare(
  fetch_stb_image_write
  URL https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
  DOWNLOAD_NO_EXTRACT ON
)

FetchContent_MakeAvailable(
  fetch_vk_bootstrap
  fetch_vma
  fetch_renderdoc_app
  fetch_stb_image_write)

# Setup header-only libraries

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE ${fetch_vma_SOURCE_DIR}/src)

add_library(renderdoc_app INTERFACE)
target_include_directories(renderdoc_app INTERFACE ${fetch_renderdoc_app_SOURCE_DIR})

add_library(stb_image_write INTERFACE)
target_include_directories(stb_image_write INTERFACE ${fetch_stb_image_write_SOURCE_DIR})
