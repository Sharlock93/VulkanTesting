characterset "ASCII"

newaction {
	trigger = "clean_up",
	description = "Clean up the generated files",
	execute = function() 
		os.rmdir("premake_files")
		os.remove("Vulkan.sln")
	end
}

workspace "Vulkan"
	architecture "x86_64"
	configurations { "Debug", "Release" }
	staticruntime "off" 
	runtime "Release" 

	-- I have included these scripts just in case, if you put
	-- glsland and validation layer under lib, this these two will build them
	-- include "premake_scripts/validation_layer.lua"
	-- include "premake_scripts/glslang.lua"

	project "Vulkan"
		kind("ConsoleApp")
		language("C")

		location("premake_files")
		includedirs { "3rd_party/", "headers/" }
		libdirs { "3rd_party/libs/glslang_libs" }

		links {
			"kernel32.lib",
			"user32.lib",
			"dbghelp.lib",
			"gdi32.lib",
			"OSDependent.lib",
			"OGLCompiler.lib",
			"glslang.lib",
			"SPIRV.lib",
			"HLSL.lib",
			"SPIRV-Tools-opt.lib",
			"SPIRV-Tools.lib",
			"GenericCodeGen.lib",
			"MachineIndependent.lib",
			"glslang-default-resource-limits.lib",
			"SPVRemapper.lib",
		}

		debugenvs{
			"DISABLE_LAYER_NV_OPTIMUS_1=1",
			"DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1=1",
			"VK_LOADER_DEBUG=warn",
			"VK_LAYER_PATH=%{wks.location}/../lib/build_validation_layer/layers/Debug/"
		}

		os.mkdir("./premake_files/shader/")
		os.copyfile("./build/shader/fragment.frag", './premake_files/shader/fragment.frag')
		os.copyfile("./build/shader/vertex.vert", './premake_files/shader/vertex.vert')
		os.copyfile("./build/shader/hello.vert", './premake_files/shader/hello.vert')

		files {"headers/*.h", "src/*.c", "build/shader/*.vert", "build/shader/*.frag"}
		removefiles {"src/opengl_window.c"}

		vpaths {
			['Headers'] = {
				'headers/*.h'
			},
			['Assets'] = {
				'build/shader/**'
			},

			['Source Files'] = {
				'src/*.c'
			}
		}

		filter { "files:src/*.c", "not files:src/main.c" }
			buildaction "None"

		filter "configurations:Debug"
			defines { "DEBUG" }
			symbols "On"

		filter "configurations:Release"
			defines { "NDEBUG" }
			optimize "On"


