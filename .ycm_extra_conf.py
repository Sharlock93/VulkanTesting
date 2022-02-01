def Settings(**kwargs):
	return {
		"flags": [
			"-I", "src/",
			"-I", "include",
			"-I", "C:/VulkanSDK/1.2.198.1/Include",
			"-target", "x86_64-pc-windows-gnu"
		],

		"override_filename" : "src/main.c",
		"file": "src/main.c"
	}
