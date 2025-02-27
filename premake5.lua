workspace "VulkanEngine"
    configurations { "Debug", "Release" }
    architecture "x64"

project "VulkanEngine"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    location "build"

    targetdir ("bin/" .. "%{cfg.buildcfg}")
    objdir ("bin-int/" .. "%{cfg.buildcfg}")

    files { "src/**.h", "src/**.cpp" }

    includedirs {
        "Libraries/GLFW/include",
        "Libraries/GLM"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        postbuildcommands {
            "call %{wks.location}src/shaders/compile.bat"
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        postbuildcommands {
            "call %{wks.location}src/shaders/compile.bat"
        }
        

-- Vulkan Detection
local vulkanSDK = os.getenv("VULKAN_SDK")

if vulkanSDK then
    print("Vulkan SDK found: " .. vulkanSDK)
    
    filter "configurations:Debug"
        includedirs { vulkanSDK .. "/Include" }
        libdirs { vulkanSDK .. "/Lib" }
        links { "vulkan-1" }

    filter "configurations:Release"
        includedirs { vulkanSDK .. "/Include" }
        libdirs { vulkanSDK .. "/Lib" }
        links { "vulkan-1" }

    defines { "HAS_VULKAN" }
else
    print("Vulkan SDK NOT found! Skipping Vulkan integration.")
end

glfwPath = path.getabsolute("Libraries/GLFW")

-- build GLFW with CMake
if not os.isfile(glfwPath .. "/build/CMakeCache.txt") then
    print("Building GLFW using CMake...")

    os.execute("mkdir \"" .. glfwPath .. "/build\"")

    os.execute("cd \"" .. glfwPath .. "/build\" && cmake .. -G \"Visual Studio 17 2022\"")
    os.execute("cd \"" .. glfwPath .. "/build\" && cmake --build . --config Debug")
    os.execute("cd \"" .. glfwPath .. "/build\" && cmake --build . --config Release")
end

-- Link GLFW built libraries
filter "configurations:Debug"
    libdirs { glfwPath .. "/build/src/Debug" }
    links { "glfw3" }

filter "configurations:Release"
    libdirs { glfwPath .. "/build/src/Release" }
    links { "glfw3" }
