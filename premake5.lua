workspace "VulkanEngine"
    configurations { "Debug", "Release" }
    architecture "x64"

project "vulkan_engine"
    location "vulkan_engine"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"
    targetdir ("bin/" .. "%{cfg.buildcfg}")
    objdir ("bin-int/" .. "%{cfg.buildcfg}")

    files { "src/**.h",
            "src/**.cpp",
            "src/tools/**.h",
            "src/tools/**.cpp",
            "src/shaders/**.vert",
            "src/shaders/**.frag"
    }

    includedirs {
        "Libraries/GLFW/include",
        "Libraries/GLM",
        "Libraries/STB"
    }

    filter "system:windows"
        systemversion "latest"
        buildoptions { "/wd4251" }  -- Disable warning C4251

    filter "configurations:Debug"
        defines { "DEBUG;_WINDOWS;_USRDLL;ENGINE_EXPORTS" }
        runtime "Debug"
        symbols "on"
        postbuildcommands {
            "call %{wks.location}src/shaders/compile.bat"
        }

    filter "configurations:Release"
        defines { "NDEBUG;_WINDOWS;_USRDLL;ENGINE_EXPORTS" }
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

        os.execute("cd \"" .. glfwPath .. "/build\" && cmake ..")
        os.execute("cd \"" .. glfwPath .. "/build\" && cmake --build . --config Debug")
        os.execute("cd \"" .. glfwPath .. "/build\" && cmake --build . --config Release")
    end

    filter "configurations:Debug"
        libdirs { glfwPath .. "/build/src/Debug" }
        links { "glfw3" }

    filter "configurations:Release"
        libdirs { glfwPath .. "/build/src/Release" }
        links { "glfw3" }


-----------------------------
project "engine_test"
    location "engine_test"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir ("bin/" .. "%{cfg.buildcfg}")
    objdir ("bin-int/" .. "%{cfg.buildcfg}")

    files {
        "test/**.h",
        "test/**.cpp"
    }

    includedirs {
        "Libraries/GLFW/include",
        "Libraries/GLM",
        "Libraries/STB",
        "./"
    }

    links {
        "vulkan_engine"
    }

    filter "configurations:Debug"
        defines { "DEBUG;_DEBUG;_WINDOWS" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG;_WINDOWS" }
        optimize "On"


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