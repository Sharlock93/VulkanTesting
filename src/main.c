#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sh_tools.c"
#include "win_platform.c"

#include "load_vulkan.c"


typedef struct sh_vulkan_pdevice {
	VkPhysicalDeviceType type;
	VkPhysicalDevice device;
	VkPhysicalDeviceFeatures features;
	VkDevice ldevice;
	VkQueueFamilyProperties *queue_properties;
} sh_vulkan_pdevice;




LRESULT sh_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) { return DefWindowProc(hwnd, msg, wparam, lparam); }
sh_window_context_t* setup() {
	sh_window_context_t *ctx = (sh_window_context_t*)calloc(1, sizeof(sh_window_context_t));

	ctx->height = 500;
	ctx->width = 500;
	ctx->x = 20;
	ctx->y = 20;
	ctx->window_name = "hello world";

	sh_create_window(ctx, sh_proc);

	return ctx;
}

char** check_layers() {

	u32 c = 0;
	vkEnumerateInstanceLayerProperties(&c, NULL);
	VkLayerProperties *p = (VkLayerProperties*) malloc(sizeof(VkLayerProperties)*c);
	vkEnumerateInstanceLayerProperties(&c, p);
	char **layer_names = NULL;
	buf_fit(layer_names, (i32)c);
	for(u32 i = 0; i < c; i++) {
		char *name = (char*) malloc(sizeof(char)*strlen(p[i].layerName) + 1);
		strcpy(name, p[i].layerName);
		buf_push(layer_names, name);
	}

	free(p);
	return layer_names;
}


char** check_instance_extensions() {

	u32 c = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &c, NULL);

	VkExtensionProperties *p = (VkExtensionProperties*) malloc(sizeof(VkExtensionProperties)*c);

	vkEnumerateInstanceExtensionProperties(NULL, &c, p);

	char **extension_names = NULL;

	buf_fit(extension_names, (i32)c);

	for(u32 i = 0; i < c; i++) {
		char *name = (char*) malloc(sizeof(char)*strlen(p[i].extensionName)+1);
		strcpy(name, p[i].extensionName);
		buf_push(extension_names, name);
	}

	free(p);
	return extension_names;
}

char** check_pdevice_extensions(sh_vulkan_pdevice *dev) {

	u32 c = 0;
	vkEnumerateDeviceExtensionProperties(dev->device, NULL, &c, NULL);

	VkExtensionProperties *p = (VkExtensionProperties*) malloc(sizeof(VkExtensionProperties)*c);

	vkEnumerateDeviceExtensionProperties(dev->device, NULL, &c, p);
	char **dev_ext_names = NULL;
	buf_fit(dev_ext_names, (i32)c);

	for(u32 i = 0; i < c; i++) {
		char *name = (char*) malloc(sizeof(char)*strlen(p[i].extensionName)+1);
		strcpy(name, p[i].extensionName);
		buf_push(dev_ext_names, name);
		// printf("%s\n", name);
	}

	free(p);
	return dev_ext_names;
}


void check_pdevice_features(sh_vulkan_pdevice *pdev) {

	// printf("%lld\n", sizeof(VK_DEVICE_FEATURE_NAMES)/sizeof(VK_DEVICE_FEATURE_NAMES[0]));
	// VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(pdev->device, &pdev->features);
	i32 feature_size = sizeof(VK_DEVICE_FEATURE_NAMES)/sizeof(VK_DEVICE_FEATURE_NAMES[0]);

	VkBool32 *f = (VkBool32*)&pdev->features;
	// printf("Features:\n");
	for(i32 i = 0; i < feature_size; i++) {
		// printf("\t%-40s = %d\n", GET_FEATURE_NAME(i), f[i]);
	}

}

i32 check_pdevice_qfamily_features(sh_vulkan_pdevice *pdev) {
	
	u32 count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pdev->device, &count, NULL);

	pdev->queue_properties = (VkQueueFamilyProperties*) calloc(count, sizeof(VkQueueFamilyProperties));

	vkGetPhysicalDeviceQueueFamilyProperties(pdev->device, &count, pdev->queue_properties);

	for(u32 i = 0; i < count; i++) {

		if(pdev->queue_properties[i].queueCount) {
			return i;
		}
		// printf("Queue Count: %d\n", pdev->queue_properties[i].queueCount);
		// sh_print_queue_family_flags(pdev->queue_properties[i].queueFlags);
	}

	return -1;
}

sh_vulkan_pdevice check_physical_devices(VkInstance *inst) {

	sh_vulkan_pdevice dev = {0};

	VkPhysicalDevice *devices = NULL;

	u32 count = 0;
	vkEnumeratePhysicalDevices(*inst, &count, NULL);

	devices = (VkPhysicalDevice*)malloc(sizeof(devices)*count);

	vkEnumeratePhysicalDevices(*inst, &count, devices);

	VkPhysicalDeviceProperties p = {0};
	for(i32 i = 0; i < (i32)count; i++) {
		vkGetPhysicalDeviceProperties(devices[i], &p);

		if(p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			dev.device = devices[i];
			dev.type = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		}

		// printf("%s[%s] %d.%d.%d\n", p.deviceName, GET_TYPE_NAME(p.deviceType), VK_API_VERSION_MAJOR(p.apiVersion), VK_API_VERSION_MINOR(p.apiVersion), VK_API_VERSION_PATCH(p.apiVersion));
	}


	free(devices);

	return dev;
}




VkInstance setup_vulkan() {

	u32 v = 0;
	vkEnumerateInstanceVersion(&v);
	// printf("%d %d %d\n", VK_VERSION_MAJOR(v), VK_VERSION_MINOR(v), VK_VERSION_PATCH(v));

	char **names = check_instance_extensions();
	char **layers = check_layers();


	// Application info
	VkApplicationInfo app_info = {0};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	app_info.pNext = NULL;
	app_info.pApplicationName = "Hello Vulkan";
	app_info.pEngineName = "sh_it";
	app_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 2, 0);

	
	VkInstanceCreateInfo create_info = {0};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = NULL;
	create_info.enabledLayerCount = 0;//buf_len(layers);
	create_info.ppEnabledLayerNames = 0;//(const char * const *)layers;

	create_info.enabledExtensionCount = buf_len(names);
	create_info.ppEnabledExtensionNames = (const char* const*)names;

	VkInstance inst = {0};

	VkResult create_instance = vkCreateInstance(&create_info, NULL, &inst);

	if(create_instance != VK_SUCCESS) {
		printf("We couldn't create vulkan instance wut");
	}

	fflush(stdout);

	load_vulkan_instance_funcs(&inst);

	sh_vulkan_pdevice dev = check_physical_devices(&inst);


	check_pdevice_features(&dev);

	char **dev_ext_names = check_pdevice_extensions(&dev);

	i32 queue_index = check_pdevice_qfamily_features(&dev);

	float qpriority = 0.5;

	VkDeviceQueueCreateInfo queue_create_info = {0};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.flags = 0;
	queue_create_info.queueFamilyIndex = queue_index;
	queue_create_info.queueCount = 1;
	queue_create_info.pQueuePriorities = &qpriority;

	VkDeviceCreateInfo device_create_info = {0};
	
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.flags = 0;
	device_create_info.pQueueCreateInfos = &queue_create_info;
	device_create_info.pEnabledFeatures = &dev.features;


	VkResult make_device = vkCreateDevice(dev.device, &device_create_info, NULL, &dev.ldevice);

	if(make_device != VK_SUCCESS) {
		printf("We couldn't create a logical device");
	}

	// Device Operations
	load_vulkan_device_funcs(&dev.ldevice);

	VkQueue q;

	vkGetDeviceQueue(dev.ldevice, queue_index, 0, &q);




	return inst;
}

int main(int argc, char **argv) {
	load_vulkan_funcs();
	setup_vulkan();
#if 0
	sh_window_context_t *ctx = setup();
	
	while(!ctx->should_close) {
		sh_handle_events(ctx);

		if(ctx->mouse->left.pressed) {
			printf("%d %d %d %d\n", ctx->mouse->x, ctx->mouse->y, ctx->mouse->left.pressed_once, ctx->mouse->right.pressed_once);
		}
	}
#endif


	return 0;
}
