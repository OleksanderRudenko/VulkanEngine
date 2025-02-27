@echo off
setlocal

cd /d "%~dp0"

if not defined VULKAN_SDK (
    echo Vulkan SDK not found. Make sure it is installed and the environment variable is set.
    exit /b 1
)

set GLSLC="%VULKAN_SDK%\Bin\glslc.exe"

if not exist %GLSLC% (
    echo glslc.exe not found in Vulkan SDK directory.
    exit /b 1
)

%GLSLC% shader.vert -o vert.spv
%GLSLC% shader.frag -o frag.spv

echo Shader compilation completed.
exit /b 0