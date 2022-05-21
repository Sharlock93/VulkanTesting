local name = "glslang"

if _ACTION ~= 'clean_up' then

	os.executef('cmake --log-level NOTICE -G %q -B%s -S%s -D %s -D %s -D %s -D %s -D %s -D %s',
	"Visual Studio 17 2022",
	"../lib/build_" .. name,
	"../lib/" .. name,
	"ENABLE_HLSL=OFF",
	"ENABLE_CTEST=OFF",
	"ENABLE_EXCEPTIONS=OFF",
	"ENABLE_GLSLANG_BINARIES=OFF",
	"ENABLE_GLSLANG_JS=OFF",
	"BUILD_TESTING=OFF"
	)
end


externalproject "glslang"
	filename "glslang/glslang"
	kind "StaticLib"
	language "C++"
	location( "../lib/build_glslang/"  )
	
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"


externalproject "spirv"
	filename "SPIRV/SPIRV"
	kind "StaticLib"
	language "C++"
	location( "../lib/build_glslang/"  )
	
	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
