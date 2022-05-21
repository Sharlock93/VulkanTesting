@echo off
pushd build
rem set VK_LOADER_DEBUG=all
rem set VK_LAYER_PATH="S:\code\projects\vulkan_gen\Vulkan-ValidationLayers\build\layers\Debug";
main.exe

echo %ERRORLEVEL%
exit 0
popd
