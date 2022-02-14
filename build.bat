@echo off

set libs_win=kernel32.lib user32.lib gdi32.lib GenericCodeGend.lib glslang-default-resource-limitsd.lib glslangd.lib HLSLd.lib MachineIndependentd.lib OGLCompilerd.lib OSDependentd.lib SPIRV-Tools-optd.lib SPIRV-Toolsd.lib SPIRVd.lib SPVRemapperd.lib
:: set inc="C:/VulkanSDK/1.2.198.1/Include/"
:: set lib_vulk="C:/VulkanSDK/1.2.198.1/Lib/"
set inc="S:\code\projects\vulkan_glsl\build\install\include"
set lib_vulk="S:\code\projects\vulkan_glsl\build\install\lib"
set compiler_ops=-W3 /Zi /MDd /EHsc
set link_ops=/LIBPATH:%lib_vulk%

cl /nologo ^
-Iinclude ^
-I %inc% ^
src\main.c ^
/Fe./build/main.exe -Fo./build/main.obj -Fd./build/main.pdb ^
%libs_win% ^
%compiler_ops% ^
/link %link_ops%
