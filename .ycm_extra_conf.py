def Settings(**kwargs):
	return {
		"flags": [
			"-I", "3rd_party/",
			"-I", "headers/",
			"-I", "lib/",
			"-I", "src/",
			"-x", "c",
			"--std","c11",
			"-DUSE_PLATFORM_WIN32_KHR",
			"-DNO_VK_PROTOTYPES"
		],

		"override_filename" : "src/main.c",
	}
