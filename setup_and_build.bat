call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
if not exist "build" mkdir "build"
set DISABLE_LAYER_NV_OPTIMUS_1=1
set DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1=1

set libs_win=user32.lib gdi32.lib OpenGL32.lib
:: set inc="C:/VulkanSDK/1.2.198.1/Include/"
:: set lib_vulk="C:/VulkanSDK/1.2.198.1/Lib/"
set inc="."
set lib_vulk="."
set compiler_ops=-W3 /Zi /MT /EHsc
set link_ops=/LIBPATH:%lib_vulk%

cl /nologo ^
-Iinclude ^
-I %inc% ^
src\main.c ^
/Fe./build/main.exe -Fo./build/main.obj -Fd./build/main.pdb ^
%libs_win% ^
%compiler_ops% ^
/link %link_ops%
