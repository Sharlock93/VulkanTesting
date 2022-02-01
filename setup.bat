call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
start /B
if not exist "build" mkdir "build"
set DISABLE_LAYER_NV_OPTIMUS_1=1
set DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1=1
gvim

