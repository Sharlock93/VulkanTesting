call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set DISABLE_LAYER_NV_OPTIMUS_1=1
set DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1=1
set VK_LOADER_DEBUG=warn
set VK_LAYER_PATH=S:\code\projects\vulkan_gen\Vulkan-ValidationLayers\build\layers\Debug
start /B
if not exist "build" mkdir "build"

gvim

