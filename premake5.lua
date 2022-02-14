characterset "ASCII"

workspace "Vulkan"
	architecture "x86_64"
	configurations { "Debug", "Release" }

location("premake_test/build")

project "Vulkan"
	kind("consoleapp")
	language("C")

location("premake_test/build")

includedirs { "include/" }


files {"src/**"}
removefiles {"src/opengl_window.c"}

links { "user32", "gdi32" }

filter "configurations:Debug"
	defines { "DEBUG" }
	symbols "On"

filter "configurations:Release"
	defines { "NDEBUG" }
	optimize "On"


filter { "files:src/**", "not files:src/main.c" }
buildaction "None"

-- filter "files:src/main.c"
-- buildaction "Compile"
