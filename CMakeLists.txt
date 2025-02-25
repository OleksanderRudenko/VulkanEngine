cmake_minimum_required(VERSION 3.20)
project(VulkanGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set project source files
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Include GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/GLFW)

# Include GLM
add_subdirectory(libraries/GLM)

# Create executable
add_executable(VulkanGame ${SOURCES})

# Include directories
target_include_directories(VulkanGame PRIVATE src libraries/GLFW/include libraries/GLM)

# Link libraries
target_link_libraries(VulkanGame PRIVATE Vulkan::Vulkan glfw glm)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(VulkanGame PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(UNIX)
    target_compile_definitions(VulkanGame PRIVATE VK_USE_PLATFORM_XCB_KHR)
endif()