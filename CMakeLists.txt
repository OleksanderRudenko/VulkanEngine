cmake_minimum_required(VERSION 3.20)
project(VulkanEngine LANGUAGES CXX)

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
add_subdirectory(Libraries/GLFW)

# Include GLM
add_subdirectory(Libraries/GLM)

# Create executable
add_executable(VulkanEngine ${SOURCES})

# Include directories
target_include_directories(VulkanEngine PRIVATE src Libraries/glfw/include/GLFW Libraries/glm/glm)

# Link libraries
target_link_libraries(VulkanEngine PRIVATE Vulkan::Vulkan glfw glm)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(VulkanEngine PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(UNIX)
    target_compile_definitions(VulkanEngine PRIVATE VK_USE_PLATFORM_XCB_KHR)
endif()