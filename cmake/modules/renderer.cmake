include(FetchContent)

FetchContent_Declare(
  fetch_vk_bootstrap
  GIT_REPOSITORY  https://github.com/charles-lunarg/vk-bootstrap.git
  GIT_TAG         v0.4
  GIT_SHALLOW     ON
  GIT_PROGRESS    ON
)

FetchContent_Declare(
  fetch_glm
  GIT_REPOSITORY  https://github.com/g-truc/glm.git
  GIT_TAG         0.9.9.8
  GIT_SHALLOW     ON
  GIT_PROGRESS    ON
)

# TODO: Try to use GitHub links instead of raw links
# WARNING: Requires latest cmake 3.21+ otherwise SOURCE_DIR is not populated
FetchContent_Declare(
  fetch_vma
  URL https://raw.githubusercontent.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/master/include/vk_mem_alloc.h
  DOWNLOAD_NO_EXTRACT ON
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
  fetch_stb_image
  URL https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
  DOWNLOAD_NO_EXTRACT ON
)

# TODO: Try to use GitHub links instead of raw links
# WARNING: Requires latest cmake 3.21+ otherwise SOURCE_DIR is not populated
FetchContent_Declare(
  fetch_stb_image_write
  URL https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
  DOWNLOAD_NO_EXTRACT ON
)

# TODO: Try to use GitHub links instead of raw links
# WARNING: Requires latest cmake 3.21+ otherwise SOURCE_DIR is not populated
FetchContent_Declare(
  fetch_nlohmann_json
  URL https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
  DOWNLOAD_NO_EXTRACT ON
  SOURCE_DIR _deps/fetch_nlohmann_json/nlohmann
)

# TODO: Try to use GitHub links instead of raw links
# WARNING: Requires latest cmake 3.21+ otherwise SOURCE_DIR is not populated
FetchContent_Declare(
  fetch_fx_gltf
  URL https://raw.githubusercontent.com/jessey-git/fx-gltf/master/include/fx/gltf.h
  DOWNLOAD_NO_EXTRACT ON
  SOURCE_DIR _deps/fetch_fx_gltf-src/fx
)

FetchContent_MakeAvailable(
  fetch_vk_bootstrap
  fetch_glm
  fetch_vma
  fetch_renderdoc_app
  fetch_stb_image
  fetch_stb_image_write
  fetch_nlohmann_json
  fetch_fx_gltf)

# Setup header-only libraries

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE ${fetch_vma_SOURCE_DIR})

add_library(renderdoc_app INTERFACE)
target_include_directories(renderdoc_app INTERFACE ${fetch_renderdoc_app_SOURCE_DIR})

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${fetch_stb_image_SOURCE_DIR})

add_library(stb_image_write INTERFACE)
target_include_directories(stb_image_write INTERFACE ${fetch_stb_image_write_SOURCE_DIR})

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${fetch_nlohmann_json_SOURCE_DIR}/..)

add_library(fx-gltf INTERFACE)
target_include_directories(fx-gltf INTERFACE ${fetch_fx_gltf_SOURCE_DIR}/..)
