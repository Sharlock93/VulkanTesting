#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include "vulkan_enum_strings.c"
#include "vulkan_extensions_to_enable.h"

#define LOAD_VK 1

#define CHECK_VK_RESULT(func_call) {\
	VkResult r = func_call;\
	if(r != VK_SUCCESS) {\
		printf("Vulkan call %s failed @%d\n", #func_call, __LINE__);\
	}\
}

#define CHECK_VK_RESULT_MSG(func_call, msg) {\
	VkResult r = func_call;\
	if(r != VK_SUCCESS) {\
		printf("Vulkan call %s failed @%d : %s\n", #func_call, __LINE__, msg);\
	}\
}

#if LOAD_VK

#define SH_VK_FUNC(name) PFN_##name name
#define SH_LOAD_VK_GET_INST_PROC_ADDR(name) name = (PFN_##name)GetProcAddress(vulkan_lib, #name);\
		if(name == NULL) { printf("Coudln't load function: %s. [%s - %d]", #name, __FILE__, __LINE__); exit(1); }

#else

#define SH_VK_FUNC(name)
#define SH_LOAD_VK_GET_INST_PROC_ADDR(name)
#define SH_FUNC_LOAD_IN(name)
#define SH_FUNC_LOAD_DEV(name)

#endif


SH_VK_FUNC(vkGetInstanceProcAddr);
SH_VK_FUNC(vkGetDeviceProcAddr);

SH_VK_FUNC(vkEnumerateInstanceVersion);
SH_VK_FUNC(vkCreateInstance);
SH_VK_FUNC(vkDestroyInstance);
SH_VK_FUNC(vkEnumerateInstanceExtensionProperties);
SH_VK_FUNC(vkEnumerateInstanceLayerProperties);

SH_VK_FUNC(vkEnumeratePhysicalDevices);
SH_VK_FUNC(vkEnumerateDeviceExtensionProperties);
SH_VK_FUNC(vkCreateDevice);
SH_VK_FUNC(vkDestroyDevice);
SH_VK_FUNC(vkGetPhysicalDeviceFeatures);
SH_VK_FUNC(vkGetPhysicalDeviceFeatures2);
SH_VK_FUNC(vkGetPhysicalDeviceProperties);
SH_VK_FUNC(vkGetPhysicalDeviceProperties2);

SH_VK_FUNC(vkGetPhysicalDeviceQueueFamilyProperties);
SH_VK_FUNC(vkGetPhysicalDeviceQueueFamilyProperties2);
SH_VK_FUNC(vkGetDeviceQueue);

SH_VK_FUNC(vkCreateDebugUtilsMessengerEXT);
SH_VK_FUNC(vkDestroyDebugUtilsMessengerEXT);

SH_VK_FUNC(vkGetPhysicalDeviceMemoryProperties2);
SH_VK_FUNC(vkGetDeviceQueue2);

SH_VK_FUNC(vkCreateWin32SurfaceKHR);
SH_VK_FUNC(vkGetPhysicalDeviceWin32PresentationSupportKHR);
SH_VK_FUNC(vkDestroySurfaceKHR);

SH_VK_FUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
SH_VK_FUNC(vkGetPhysicalDeviceSurfaceCapabilities2KHR);
SH_VK_FUNC(vkGetPhysicalDeviceSurfaceFormatsKHR);
SH_VK_FUNC(vkGetPhysicalDeviceSurfaceFormats2KHR);
SH_VK_FUNC(vkGetPhysicalDeviceSurfacePresentModesKHR);
SH_VK_FUNC(vkGetPhysicalDeviceSurfaceSupportKHR);
SH_VK_FUNC(vkGetPhysicalDeviceSurfacePresentModes2EXT);


SH_VK_FUNC(vkAcquireNextImageKHR);
SH_VK_FUNC(vkCreateSwapchainKHR);
SH_VK_FUNC(vkDestroySwapchainKHR);
SH_VK_FUNC(vkGetSwapchainImagesKHR);
SH_VK_FUNC(vkQueuePresentKHR);

SH_VK_FUNC(vkAcquireNextImage2KHR);
SH_VK_FUNC(vkGetDeviceGroupPresentCapabilitiesKHR);
SH_VK_FUNC(vkGetDeviceGroupSurfacePresentModesKHR);
SH_VK_FUNC(vkGetPhysicalDevicePresentRectanglesKHR);

SH_VK_FUNC(vkEnumeratePhysicalDeviceGroups);

SH_VK_FUNC(vkCreateImageView);
SH_VK_FUNC(vkDestroyImageView);

SH_VK_FUNC(vkCreateShaderModule);
SH_VK_FUNC(vkDestroyShaderModule);

SH_VK_FUNC(vkCreatePipelineLayout);

#undef SH_VK_FUNC


typedef struct sh_vulkan_pdevice {
	VkPhysicalDeviceType type;
	VkPhysicalDevice device;
	VkPhysicalDeviceFeatures2 *features;
	VkPhysicalDeviceProperties2 *properties;
	VkDevice ldevice;
	VkQueueFamilyProperties2 *queue_properties;
} sh_vulkan_pdevice;


typedef struct sh_vk_shader_module {
	VkShaderModule shader_module;
	glslang_stage_t stage;
} sh_vk_shader_module;

typedef struct sh_vulkan_context_t {
	sh_vulkan_pdevice device_info;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_msgr;
	i32 queue_family_selected;
	const char** layers_enabled;
	const char** extensions_enabled;
	const char** pdevice_extensions_enabled;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkImageView *img_views;
	sh_vk_shader_module *shader_modules;
} sh_vulkan_context_t;

typedef struct sh_vk_spirv_shader_t {
	u32 *data;	
	u64 size;
} sh_vk_spirv_shader_t;

void load_vulkan_funcs(void) {
	HMODULE vulkan_lib = LoadLibrary("vulkan-1.dll");
	// printf("%p\n", vulkan_lib);

	SH_LOAD_VK_GET_INST_PROC_ADDR(vkGetInstanceProcAddr);
	SH_LOAD_VK_GET_INST_PROC_ADDR(vkGetDeviceProcAddr);

#define SH_VK_FUNC(name) name = (PFN_##name)vkGetInstanceProcAddr(NULL, #name);\
		if(name == NULL) { printf("Coudln't load vulkan global function: %s. [%s - %d]", #name, __FILE__, __LINE__); exit(1); }


	SH_VK_FUNC(vkCreateInstance);
	SH_VK_FUNC(vkEnumerateInstanceExtensionProperties);
	SH_VK_FUNC(vkEnumerateInstanceLayerProperties);
	SH_VK_FUNC(vkEnumerateInstanceVersion);

#undef SH_VK_FUNC

}


void load_vulkan_instance_funcs(VkInstance *inst) {

#define SH_VK_FUNC(name) name = (PFN_##name)vkGetInstanceProcAddr(*(inst), #name);\
	if(name == NULL) { printf("Coudln't load vulkan instance function: %s. [%s - %d]", #name, __FILE__, __LINE__); exit(1); }


	SH_VK_FUNC(vkEnumeratePhysicalDevices);
	SH_VK_FUNC(vkGetPhysicalDeviceProperties);
	SH_VK_FUNC(vkGetPhysicalDeviceProperties2);
	SH_VK_FUNC(vkGetPhysicalDeviceFeatures);
	SH_VK_FUNC(vkGetPhysicalDeviceFeatures2);

	SH_VK_FUNC(vkDestroyInstance);

	SH_VK_FUNC(vkCreateDevice);
	SH_VK_FUNC(vkDestroyDevice);
	SH_VK_FUNC(vkEnumerateDeviceExtensionProperties);

	SH_VK_FUNC(vkCreateDebugUtilsMessengerEXT);
	SH_VK_FUNC(vkDestroyDebugUtilsMessengerEXT);

	SH_VK_FUNC(vkGetPhysicalDeviceQueueFamilyProperties);
	SH_VK_FUNC(vkGetPhysicalDeviceQueueFamilyProperties2);

	SH_VK_FUNC(vkGetPhysicalDeviceMemoryProperties2);

	SH_VK_FUNC(vkCreateWin32SurfaceKHR);
	SH_VK_FUNC(vkGetPhysicalDeviceWin32PresentationSupportKHR);
	SH_VK_FUNC(vkDestroySurfaceKHR);

	SH_VK_FUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
	SH_VK_FUNC(vkGetPhysicalDeviceSurfaceCapabilities2KHR);
	SH_VK_FUNC(vkGetPhysicalDeviceSurfaceFormatsKHR);
	SH_VK_FUNC(vkGetPhysicalDeviceSurfaceFormats2KHR);
	SH_VK_FUNC(vkGetPhysicalDeviceSurfacePresentModesKHR);
	SH_VK_FUNC(vkGetPhysicalDeviceSurfacePresentModes2EXT);
	SH_VK_FUNC(vkGetPhysicalDeviceSurfaceSupportKHR);
	SH_VK_FUNC(vkGetPhysicalDevicePresentRectanglesKHR);

	SH_VK_FUNC(vkEnumeratePhysicalDeviceGroups);


#undef SH_VK_FUNC

}

void load_vulkan_device_funcs(VkDevice *dev) {
#define SH_VK_FUNC(name) name = (PFN_##name)vkGetDeviceProcAddr(*(dev), #name);\
		if(name == NULL) { printf("Coudln't load vulkan device function: %s. [%s - %d]", #name, __FILE__, __LINE__); exit(1); }

	SH_VK_FUNC(vkGetDeviceQueue);
	SH_VK_FUNC(vkGetDeviceQueue2);
	SH_VK_FUNC(vkDestroyDevice);

	SH_VK_FUNC(vkAcquireNextImageKHR);
	SH_VK_FUNC(vkCreateSwapchainKHR);
	SH_VK_FUNC(vkDestroySwapchainKHR);
	SH_VK_FUNC(vkGetSwapchainImagesKHR);
	SH_VK_FUNC(vkQueuePresentKHR);

	SH_VK_FUNC(vkAcquireNextImage2KHR);
	SH_VK_FUNC(vkGetDeviceGroupPresentCapabilitiesKHR);
	SH_VK_FUNC(vkGetDeviceGroupSurfacePresentModesKHR);

	SH_VK_FUNC(vkCreateImageView);
	SH_VK_FUNC(vkDestroyImageView);

	SH_VK_FUNC(vkCreateShaderModule);
	SH_VK_FUNC(vkDestroyShaderModule);

	SH_VK_FUNC(vkCreatePipelineLayout);
#undef SH_VK_FUNC

}




char** check_layers(void) {
	char** names = NULL;

	if(ARRAY_SIZE(gl_layers_to_enable) == 1 && gl_layers_to_enable[0] == NULL) { return NULL;}

#if ENABLE_LAYERS
	u32 c = 0;
	CHECK_VK_RESULT(vkEnumerateInstanceLayerProperties(&c, NULL));

	VkLayerProperties *p = (VkLayerProperties*) malloc(sizeof(VkLayerProperties)*c);
	vkEnumerateInstanceLayerProperties(&c, p);

	buf_fit(names, c);

	for(u32 i = 0; i < c; i++) {
		// printf("%s\n", p[i].layerName);
		for(i32 j = 0; j < ARRAY_SIZE(gl_layers_to_enable); j++) {
			if(strcmp(p[i].layerName, gl_layers_to_enable[j]) == 0) {
				buf_push(names, gl_layers_to_enable[j]);
			}
		}
	}

	if(buf_len(names) != ARRAY_SIZE(gl_layers_to_enable)) {
		printf("These layers were not found\n");

		i32 found = 0;
		for(i32 i = 0; i < ARRAY_SIZE(gl_layers_to_enable); i++) {
			found = 0;
			for(u32 j = 0; j < buf_len(names); j++) {
				if(strcmp(names[j], gl_layers_to_enable[i]) == 0) {
					found = 1;
					break;
				}
			}

			if(found == 0) {
				printf("%s\n", gl_layers_to_enable[i]);
			}
		}

	}


	free(p);
#endif 
	return names;
}


const char** check_instance_extensions(void) {

	char** names = NULL;


	if(ARRAY_SIZE(gl_instance_extensions_to_enable) == 1 &&
			gl_instance_extensions_to_enable[0] == NULL)
	{ return NULL;}

#if ENABLE_EXTENSIONS

	u32 c = 0;
	CHECK_VK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &c, NULL));
	VkExtensionProperties *p = (VkExtensionProperties*) malloc(sizeof(VkExtensionProperties)*c);
	CHECK_VK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &c, p));


	buf_fit(names, c);

	for(u32 i = 0; i < c; i++) {
		// printf("%s\n", p[i].extensionName);
		for(i32 j = 0; j < ARRAY_SIZE(gl_instance_extensions_to_enable); j++) {
			if(strcmp(p[i].extensionName, gl_instance_extensions_to_enable[j]) == 0) {
				buf_push(names, gl_instance_extensions_to_enable[j]);
			}
		}
	}

	if(buf_len(names) != ARRAY_SIZE(gl_instance_extensions_to_enable)) {
		printf("These extensions were not found\n");
		i32 found = 0;
		for(i32 i = 0; i < ARRAY_SIZE(gl_instance_extensions_to_enable); i++) {
			found = 0;
			for(u32 j = 0; j < buf_len(names); j++) {
				if(strcmp(names[j], gl_instance_extensions_to_enable[i]) == 0) {
					found = 1;
					break;
				}
			}
			if(found == 0) { printf("%s\n", gl_instance_extensions_to_enable[i]); }
		}
	}


	free(p);

#endif
	return names;
}


void check_physical_devices(sh_vulkan_context_t *vk_ctx) {

	VkPhysicalDevice *devices = NULL;
	u32 count = 0;
	CHECK_VK_RESULT_MSG(
			vkEnumeratePhysicalDevices(vk_ctx->instance, &count, NULL),
			"Couldn't Enumerate devices"
	);

	devices = (VkPhysicalDevice*)malloc(sizeof(devices)*count);

	CHECK_VK_RESULT_MSG(
			vkEnumeratePhysicalDevices( vk_ctx->instance, &count, devices),
			"Couldn't get physical devices"
	);

	VkPhysicalDeviceProperties2 *p = (VkPhysicalDeviceProperties2*) calloc(1, sizeof(VkPhysicalDeviceProperties2));

	p->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

	printf("We found %d devices.\n", count);

	VkPhysicalDevice selected_device = VK_NULL_HANDLE;
	VkPhysicalDeviceType selected_device_type = 0;

	//TODO(sharo): could use a better approach to select a device
	for(i32 i = 0; i < (i32)count; i++) {
		vkGetPhysicalDeviceProperties2(devices[i], p);
		printf("\t");
		sh_print_pdevice_info(p);

		// As long as we have a discrete gpu, select that
		if(p->properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			selected_device = devices[i];
			selected_device_type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		}
	}

	//if no discrete GPUs see if there is an integrated one
	if(selected_device == VK_NULL_HANDLE) {
		for(i32 i = 0; i < (i32)count; i++) {
			// As long as we have a discrete gpu, select that
			if(p->properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
				selected_device = devices[i];
				selected_device_type = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
			}
		}
	}


	if(selected_device != VK_NULL_HANDLE) {

		printf("Selected Device: \n\t");

		vk_ctx->device_info.device = selected_device;
		vk_ctx->device_info.type = selected_device_type;

		vkGetPhysicalDeviceProperties2(vk_ctx->device_info.device, p);
		sh_print_pdevice_info(p);
		vk_ctx->device_info.properties = p;

		VkPhysicalDeviceFeatures2 *feat = (VkPhysicalDeviceFeatures2 *)calloc(1, sizeof(VkPhysicalDeviceFeatures2));
		feat->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		vkGetPhysicalDeviceFeatures2(vk_ctx->device_info.device, feat);
		vk_ctx->device_info.features = feat;

		// print_pdevice_feature(&feat);
	}

	free(devices);
}

const char** check_pdevice_extensions(sh_vulkan_pdevice *dev) {

	char **names = NULL;

	if(
			ARRAY_SIZE(gl_physical_device_extensions_to_enable) == 1 &&
			gl_physical_device_extensions_to_enable[0] == NULL) {
		return NULL;
	}

#if ENABLE_PDEVICE_EXTENSIONS

	u32 c = 0;
	CHECK_VK_RESULT( vkEnumerateDeviceExtensionProperties(dev->device, NULL, &c, NULL) );
	VkExtensionProperties *p = (VkExtensionProperties*) malloc(sizeof(VkExtensionProperties)*c);
	CHECK_VK_RESULT(vkEnumerateDeviceExtensionProperties(dev->device, NULL, &c, p));

	buf_fit(names, c);

	for(u32 i = 0; i < c; i++) {
		// printf("%s\n", p[i].extensionName);
		for(i32 j = 0; j < ARRAY_SIZE(gl_physical_device_extensions_to_enable); j++) {
			if(strcmp(p[i].extensionName, gl_physical_device_extensions_to_enable[j]) == 0) {
				buf_push(names, gl_physical_device_extensions_to_enable[j]);
			}
		}
	}

	if(buf_len(names) != ARRAY_SIZE(gl_physical_device_extensions_to_enable)) {

		printf("These physical device extensions were not found\n");

		i32 found = 0;
		for(i32 i = 0; i < ARRAY_SIZE(gl_physical_device_extensions_to_enable); i++) {
			found = 0;
			for(u32 j = 0; j < buf_len(names); j++) {
				if(strcmp(names[j], gl_physical_device_extensions_to_enable[i]) == 0) {
					found = 1;
					break;
				}
			}
			if(found == 0) { printf("%s\n", gl_physical_device_extensions_to_enable[i]); }
		}
	}


	free(p);

#endif

	return names;
}


void check_pdevice_qfamily_features(sh_vulkan_context_t *vk_ctx, VkQueueFlagBits flags_wanted) {
	
	sh_vulkan_pdevice *pdev = &vk_ctx->device_info;

	u32 count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2(pdev->device, &count, NULL);
	pdev->queue_properties = (VkQueueFamilyProperties2*) calloc(count, sizeof(VkQueueFamilyProperties2));

	for(u32 i = 0; i < count; i++) {
		pdev->queue_properties[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
	}

	vkGetPhysicalDeviceQueueFamilyProperties2(pdev->device, &count, pdev->queue_properties);

	i32 selected_queue_family_index = -1;

	for(u32 i = 0; i < count; i++) {
		u32 qcount =  pdev->queue_properties[i].queueFamilyProperties.queueCount;
		VkQueueFlagBits flags = pdev->queue_properties[i].queueFamilyProperties.queueFlags;
		// printf("Queue Count: %d\n", pdev->queue_properties[i].queueFamilyProperties.queueCount);
		// sh_print_queue_family_flags(flags);

		if((( flags & flags_wanted) == flags_wanted) && qcount > 1) {
			selected_queue_family_index = i;

		}
	}

	if(selected_queue_family_index != -1) {
		printf("We have selected the queue family index %d: \n", selected_queue_family_index);

		u32 qcount =  pdev->queue_properties[selected_queue_family_index].queueFamilyProperties.queueCount;
		VkQueueFlagBits flags = pdev->queue_properties[selected_queue_family_index].queueFamilyProperties.queueFlags;
		printf("\tQueue Count: %d\n",
				pdev->queue_properties[selected_queue_family_index].queueFamilyProperties.queueCount
				);
		sh_print_queue_family_flags(flags);

		vk_ctx->queue_family_selected = selected_queue_family_index;

	}

	// return -1;
}


void check_pdevice_memory(sh_vulkan_pdevice *dev) {
	VkPhysicalDeviceMemoryProperties2 device_mem = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2
	};
	vkGetPhysicalDeviceMemoryProperties2(dev->device, &device_mem);
	if(&device_mem != VK_NULL_HANDLE) {
		sh_print_pdevice_memory_properties(&device_mem.memoryProperties);
	}
}


