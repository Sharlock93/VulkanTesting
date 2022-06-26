@echo off

set libs_win= ^
kernel32.lib ^
user32.lib ^
dbghelp.lib ^
gdi32.lib ^
OSDependent.lib ^
OGLCompiler.lib ^
glslang.lib ^
SPIRV.lib ^
HLSL.lib ^
SPIRV-Tools-opt.lib ^
SPIRV-Tools.lib ^
GenericCodeGen.lib ^
MachineIndependent.lib ^
glslang-default-resource-limits.lib ^
SPVRemapper.lib

set third_party_headers="./3rd_party/"
set third_party_libs="./3rd_party/libs/"
set compiler_ops= -W3 /wd5105 /O2 /MD /EHsc /std:c11
set link_ops=/LIBPATH:%third_party_libs% /LIBPATH:"3rd_party/libs/glslang_libs/"

cl /nologo ^
-I %third_party_headers% ^
-I headers ^
src\main.c ^
/Fe./build/main.exe -Fo./build/main.obj -Fd./build/main.pdb ^
%libs_win% ^
%compiler_ops% ^
/link %link_ops%
