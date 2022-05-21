externalproject "validation_layer"
	filename "VkLayer_khronos_validation-json"
	kind "StaticLib"
	language "C++"
	targetdir "../lib/build_validation_layer/"
	location "../lib/build_validation_layer/layers/"

	includedirs {"../lib/validation_layer/layers/"}

	vpaths {

		['Header Files'] = {
			"../lib/validation_layer/layers/**.h"
		},

		["Source Files"] = {
			"../lib/validation_layer/layers/**.c"
		}
	}

	if _ACTION ~= 'clean_up' then
		
		os.executef('cmake --log-level NOTICE -G %q -B%s -S%s -D %s -D %s',
		"Visual Studio 17 2022",
		"../lib/build_validation_layer/",
		"../lib/validation_layer/",
		"UPDATE_DEPS=ON",
		"CMAKE_BUILD_TYPE=Debug"
		)
	end

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
