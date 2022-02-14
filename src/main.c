#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#include <string.h>
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "sh_tools.c"
#include "win_platform.c"

#define ENABLE_LAYERS 1
#define ENABLE_EXTENSIONS 1
#define ENABLE_PDEVICE_EXTENSIONS 1

#include <glslang/Include/glslang_c_interface.h>
#include "load_vulkan.c"



LRESULT sh_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) { return DefWindowProc(hwnd, msg, wparam, lparam); }
sh_window_context_t* setup() {
	sh_window_context_t *ctx = (sh_window_context_t*)calloc(1, sizeof(sh_window_context_t));

	ctx->width = 1920;
	ctx->height = 1080;
	ctx->x = 0;
	ctx->y = 0;
	ctx->window_name = "hello world";

	sh_create_window(ctx, (WNDPROC)sh_proc);

	return ctx;
}


VKAPI_ATTR VkBool32 VKAPI_CALL debug_dump_everything(
		VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT *data,
		void *user_data) {
	printf("[DBG][%d] %s: %s\n", data->messageIdNumber, data->pMessageIdName, data->pMessage );
	return VK_FALSE;
}



VkBool32 VKAPI_PTR create_instance_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT *data,
		void *user_data) {
	printf("[IC][%d] %s: %s\n", data->messageIdNumber, data->pMessageIdName, data->pMessage );
	return VK_FALSE;
}

void setup_debug_callbacks(sh_vulkan_context_t *vk_ctx) {

	VkDebugUtilsMessengerCreateInfoEXT debug_ext_create_instance = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debug_dump_everything,
	};


	CHECK_VK_RESULT(
		vkCreateDebugUtilsMessengerEXT(
			vk_ctx->instance,
			&debug_ext_create_instance,
			NULL,
			&vk_ctx->debug_msgr
		)
	);


}

sh_vulkan_context_t setup_vulkan() {

	sh_vulkan_context_t context = {0};
	context.queue_family_selected = -1;

	u32 v = 0;
	CHECK_VK_RESULT( vkEnumerateInstanceVersion(&v) );
	printf("Vulkan Version: %d.%d.%d\n", VK_VERSION_MAJOR(v), VK_VERSION_MINOR(v), VK_VERSION_PATCH(v));

	const char **layer_to_enable_names = check_layers();
	const char **extension_to_enable_names = check_instance_extensions();

	context.layers_enabled = layer_to_enable_names;
	context.extensions_enabled = extension_to_enable_names;

	// Application info
	VkApplicationInfo app_info = {0};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	app_info.pApplicationName = "Hello Vulkan";
	app_info.pEngineName = "sh_it";
	app_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

	VkDebugUtilsMessengerCreateInfoEXT debug_ext_create_instance = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pNext = NULL,
		.flags = 0,
		.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT    |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = create_instance_debug_callback,
		.pUserData = NULL
	};

	VkInstanceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = &debug_ext_create_instance,
		.pApplicationInfo = NULL,
		.enabledLayerCount = buf_len(layer_to_enable_names),
		.ppEnabledLayerNames = layer_to_enable_names,
		.enabledExtensionCount = buf_len(extension_to_enable_names),
		.ppEnabledExtensionNames = extension_to_enable_names
	};

	
	CHECK_VK_RESULT_MSG(
		vkCreateInstance(&create_info, NULL, &context.instance),
		"We couldn't create a VkInstance"
	);

	load_vulkan_instance_funcs(&context.instance);
	setup_debug_callbacks(&context);


	//Setup devices
	check_physical_devices(&context);
	if(context.device_info.device == VK_NULL_HANDLE) {
		printf("No physical devices availabe that pass our requirement, can't continue vulkan setup.\n");
		return context;
	}

	context.pdevice_extensions_enabled = check_pdevice_extensions(&context.device_info);
	check_pdevice_memory(&context.device_info);

	check_pdevice_qfamily_features(&context, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);


	if(context.queue_family_selected == -1) {
		printf("We couldn't select a queue family that matched out flags. ");
		return context;
	}

	float qpriority = 0.5;
	VkDeviceQueueCreateInfo queue_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.flags = 0,
		.queueFamilyIndex = context.queue_family_selected,
		.queueCount = 1,
		.pQueuePriorities = &qpriority
	};

	VkDeviceCreateInfo device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.flags = 0,
		.pQueueCreateInfos = &queue_create_info,
		.queueCreateInfoCount = 1,
		.ppEnabledExtensionNames = context.pdevice_extensions_enabled,
		.enabledExtensionCount = buf_len(context.pdevice_extensions_enabled),
		.pEnabledFeatures = &context.device_info.features->features
	};

	CHECK_VK_RESULT_MSG(
		vkCreateDevice(context.device_info.device,
			&device_create_info,
			NULL,
			&context.device_info.ldevice
		),
		"We couldn't create a logical device"
	);

	// Device Operations
	load_vulkan_device_funcs(&context.device_info.ldevice);

	VkQueue q;

	VkDeviceQueueInfo2 device_queue_info =  {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
		.pNext = NULL,
		.flags = 0,
		.queueFamilyIndex = context.queue_family_selected,
		.queueIndex = 0
	};

	vkGetDeviceQueue2(context.device_info.ldevice, &device_queue_info, &q);

	if(q == VK_NULL_HANDLE) {
		printf("Couldn't create Queue ");
	}



	return context;
}

void check_physical_device_groups(sh_vulkan_context_t *vk_ctx) {
	u32 count = 0;
	CHECK_VK_RESULT( vkEnumeratePhysicalDeviceGroups(vk_ctx->instance, &count, NULL));

	printf("%d\n", count);

	VkPhysicalDeviceGroupProperties *device_groups = (VkPhysicalDeviceGroupProperties*)calloc(1, sizeof(VkPhysicalDeviceGroupProperties));

	for(u32 i = 0; i < count; i++) {
		device_groups[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
	}

	CHECK_VK_RESULT( vkEnumeratePhysicalDeviceGroups(vk_ctx->instance, &count, device_groups));

	for(u32  i = 0; i < count; i++) {
		printf("[%d] - device count: %d\n", i,  device_groups[i].physicalDeviceCount);
	}

	free(device_groups);

}

void setup_surface_and_rendering(sh_window_context_t *wn, sh_vulkan_context_t *vk_ctx) {
	// Surface creation

	VkWin32SurfaceCreateInfoKHR surface_create_info = {
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.pNext = 0,
		.flags = 0,
		.hinstance = (HINSTANCE)GetModuleHandle(NULL),
		.hwnd = wn->handle
	};

	CHECK_VK_RESULT_MSG(
			vkCreateWin32SurfaceKHR(vk_ctx->instance, &surface_create_info, NULL, &vk_ctx->surface),
			"Couldn't create surface"
	);


	VkSurfaceCapabilities2KHR surface_cap = {
		.sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR
	};

	VkPhysicalDeviceSurfaceInfo2KHR surface_inf = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR,
		.pNext = NULL,
		.surface = vk_ctx->surface
	};

	CHECK_VK_RESULT(
		vkGetPhysicalDeviceSurfaceCapabilities2KHR(
			vk_ctx->device_info.device, &surface_inf, &surface_cap
		)
	);

	sh_print_surface_capabilities(&surface_cap.surfaceCapabilities);

	u32 surface_format_count = 0;

	CHECK_VK_RESULT(
			vkGetPhysicalDeviceSurfaceFormats2KHR(vk_ctx->device_info.device, &surface_inf, &surface_format_count, NULL)
			);

	VkSurfaceFormat2KHR *surface_formats = (VkSurfaceFormat2KHR *)calloc(surface_format_count, sizeof(VkSurfaceFormat2KHR));
	for(u32 i = 0; i < surface_format_count; i++) {
		surface_formats[i].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
	}
	CHECK_VK_RESULT(
			vkGetPhysicalDeviceSurfaceFormats2KHR(vk_ctx->device_info.device, &surface_inf, &surface_format_count, surface_formats)
			);


	printf("Number for Surface Formats: %d\n", surface_format_count);
	puts("Surface Formats:");
	for(u32 i = 0; i < surface_format_count; i++) {
		printf("\t[%d] %-35s - %s\n", i,
				get_vk_format_flags_name(surface_formats[i].surfaceFormat.format),
				get_vk_surface_format_color_space_name(surface_formats[i].surfaceFormat.colorSpace)
				);
	}

	free(surface_formats);

	u32 surface_present_mode = 0;
	CHECK_VK_RESULT(
			vkGetPhysicalDeviceSurfacePresentModes2EXT(
				vk_ctx->device_info.device,
				&surface_inf,
				&surface_present_mode,
				NULL)
			);

	
	VkPresentModeKHR *present_modes = (VkPresentModeKHR*)calloc(surface_present_mode, sizeof(VkPresentModeKHR));
	CHECK_VK_RESULT(
			vkGetPhysicalDeviceSurfacePresentModes2EXT(
				vk_ctx->device_info.device,
				&surface_inf,
				&surface_present_mode,
				present_modes)
			);
	printf("Surface Present Modes [%d]\n", surface_present_mode);
	for(u32 i = 0; i < surface_present_mode; i++) {
		printf("\t[%d] %s\n", i, get_vk_present_mode_name(present_modes[i]));
	}

	free(present_modes);

	VkBool32 present_supported = 0;

	 CHECK_VK_RESULT(
		vkGetPhysicalDeviceSurfaceSupportKHR(vk_ctx->device_info.device,
			vk_ctx->queue_family_selected,
			vk_ctx->surface,
			&present_supported
		)
	);

	 printf("Presentation Supported: %s\n", present_supported ? "Yes" : "No" );

	 VkSwapchainCreateInfoKHR swapchain_create = {
		 .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		 .pNext = NULL,
		 .flags = 0,
		 .surface = vk_ctx->surface,
		 .minImageCount = 3,
		 .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
		 .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		 .imageExtent = { wn->width, wn->height },
		 .imageArrayLayers = 1,
		 .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			 		   VK_IMAGE_USAGE_TRANSFER_DST_BIT     |
					   VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		 .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		 .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		 .queueFamilyIndexCount = 0,
		 .pQueueFamilyIndices = NULL,
		 .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		 .presentMode = VK_PRESENT_MODE_MAILBOX_KHR,
		 .clipped = VK_TRUE,
		 .oldSwapchain = VK_NULL_HANDLE
	 };


	 CHECK_VK_RESULT(
		vkCreateSwapchainKHR(vk_ctx->device_info.ldevice, &swapchain_create, NULL, &vk_ctx->swapchain)
	 );


	 u32 image_count = 0;
	 CHECK_VK_RESULT(vkGetSwapchainImagesKHR(vk_ctx->device_info.ldevice, vk_ctx->swapchain, &image_count, NULL));
	 VkImage *ims = (VkImage*)calloc(image_count, sizeof(VkImage));
	 CHECK_VK_RESULT(vkGetSwapchainImagesKHR(vk_ctx->device_info.ldevice, vk_ctx->swapchain, &image_count, ims));

	 printf("Image count: %d\n", image_count);
	 for(u32 i = 0; i < image_count; i++) {
		 printf("\t[%d] %p - %p\n", i, ims + i, ims[i]);
	 }

	 

	 buf_fit(vk_ctx->img_views, image_count);

	 VkImageViewCreateInfo imv_create_infos = {
		 .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		 .pNext = NULL,
		 .flags = 0,
		 .viewType = VK_IMAGE_VIEW_TYPE_2D,
		 .format = VK_FORMAT_B8G8R8A8_SRGB,
		 .components.r = VK_COMPONENT_SWIZZLE_B,
		 .components.b = VK_COMPONENT_SWIZZLE_R
	 };

	 imv_create_infos.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	 imv_create_infos.subresourceRange.baseMipLevel = 0;
	 imv_create_infos.subresourceRange.levelCount = 1;
	 imv_create_infos.subresourceRange.baseArrayLayer = 0;
	 imv_create_infos.subresourceRange.layerCount = 1;

	 for(u32 i = 0; i < image_count; i++) {
		 imv_create_infos.image = ims[i];
		 CHECK_VK_RESULT(vkCreateImageView(vk_ctx->device_info.ldevice, &imv_create_infos, NULL, vk_ctx->img_views + i));
	 }


}

void destroy_vulkan_instance(sh_vulkan_context_t *vk_ctx) {

	for(u32 i = 0; i < buf_cap(vk_ctx->shader_modules); i++) {
		vkDestroyShaderModule( vk_ctx->device_info.ldevice, vk_ctx->shader_modules[i].shader_module, NULL);
	}

	for(u32 i = 0; i < buf_cap(vk_ctx->img_views); i++) {
		vkDestroyImageView(vk_ctx->device_info.ldevice, vk_ctx->img_views[i], NULL);
	}

	vkDestroySwapchainKHR(vk_ctx->device_info.ldevice, vk_ctx->swapchain, NULL);
	vkDestroySurfaceKHR(vk_ctx->instance, vk_ctx->surface, NULL);
	vkDestroyDevice(vk_ctx->device_info.ldevice, NULL);
	vkDestroyDebugUtilsMessengerEXT(vk_ctx->instance, vk_ctx->debug_msgr, NULL);
	vkDestroyInstance(vk_ctx->instance, NULL);
}



void struct_type_check(void) {
#define STRUCT_NAME_SIZE(field, struct) printf("Struct: %s->%s s:%lld\n", #field, #struct, sizeof(struct))

	STRUCT_NAME_SIZE(type, VkPhysicalDeviceType);
	STRUCT_NAME_SIZE(device, VkPhysicalDevice);
	STRUCT_NAME_SIZE(*features, VkPhysicalDeviceFeatures2*);
	STRUCT_NAME_SIZE(*properties, VkPhysicalDeviceProperties2*);
	STRUCT_NAME_SIZE(ldevice, VkDevice);
	STRUCT_NAME_SIZE(*queue_properties, VkQueueFamilyProperties2*);
	STRUCT_NAME_SIZE(NULL, sh_vulkan_pdevice);
	STRUCT_NAME_SIZE(NULL, sh_vulkan_context_t);

#undef STRUCT_NAME_SIZE

}

glsl_include_result_t* includesystem(void* ctx,
		const char* header_name,
		const char* includer_name,
		size_t include_depth) {

	printf("hello");
	fflush(stdout);

	return NULL;
}

glsl_include_result_t* includelocal(void* ctx,
		const char* header_name,
		const char* includer_name,
		size_t include_depth) {

	printf("hello");
	fflush(stdout);

	return NULL;
}

void compile_and_generate_shader(const glslang_input_t *shader, sh_vk_spirv_shader_t *spirv_shader ) {

	glslang_program_t *program = glslang_program_create();
	glslang_shader_t *shad = glslang_shader_create(shader);

	const char *x = glslang_shader_get_info_debug_log(shad);

	if(!glslang_shader_preprocess(shad, shader)) {
		puts("Couldn't do shader preprocessing..");
		printf("%s\n", glslang_shader_get_info_log(shad));
	}

	if(!glslang_shader_parse(shad, shader)) {
		puts("Coudln't parse shader.");
		printf("%s\n", glslang_shader_get_info_log(shad));
	}

	glslang_program_add_shader(program, shad);


	if(!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
		puts("We coudln't link the program.");
		printf("Error:\n %s\n", glslang_program_get_info_log(program));
	}

	glslang_program_SPIRV_generate(program, shader->stage);

	if(glslang_program_SPIRV_get_messages(program)) {
		puts("generating spir-v failed");
		printf("%s\n", glslang_program_SPIRV_get_messages(program));
		printf("%s\n", glslang_program_get_info_debug_log(program));
	}



	size_t data_size_in_bytes = glslang_program_SPIRV_get_size(program)*sizeof(u32);
	spirv_shader->data = (u32*)calloc(data_size_in_bytes, sizeof(char));
	spirv_shader->data = 
	memcpy(spirv_shader->data, glslang_program_SPIRV_get_ptr(program), data_size_in_bytes);
	spirv_shader->size = data_size_in_bytes;
	glslang_program_delete(program);
}
void setup_graphics_pipeline(sh_window_context_t *wn_ctx, sh_vulkan_context_t *vk_ctx) {

	char* shader_vert_source = sh_read_file("shader/vertex.vert", NULL);
	char* shader_frag_source = sh_read_file("shader/fragment.frag", NULL);

	const glslang_input_t inputs[] = { {
			.language = GLSLANG_SOURCE_GLSL,
			.stage = GLSLANG_STAGE_VERTEX,
			.client = GLSLANG_CLIENT_VULKAN,
			.client_version = GLSLANG_TARGET_VULKAN_1_3,
			.target_language = GLSLANG_TARGET_SPV,
			.target_language_version = GLSLANG_TARGET_SPV_1_3,
			.code = shader_vert_source,
			.default_version = 450,
			.default_profile = GLSLANG_NO_PROFILE,
			.force_default_version_and_profile = 0,
			.forward_compatible = 0,
			.messages = GLSLANG_MSG_DEFAULT_BIT,
			.resource = &sh_vk_default_resource_limits
		},
		  {
			.language = GLSLANG_SOURCE_GLSL,
			.stage = GLSLANG_STAGE_FRAGMENT,
			.client = GLSLANG_CLIENT_VULKAN,
			.client_version = GLSLANG_TARGET_VULKAN_1_1,
			.target_language = GLSLANG_TARGET_SPV,
			.target_language_version = GLSLANG_TARGET_SPV_1_3,
			.default_version = 450,
			.default_profile = GLSLANG_NO_PROFILE,
			.code = shader_frag_source,
			.force_default_version_and_profile = 0,
			.forward_compatible = 0,
			.messages = GLSLANG_MSG_DEFAULT_BIT,
			.resource = &sh_vk_default_resource_limits
		}
	};

	glslang_initialize_process();

	sh_vk_spirv_shader_t shaders[2] = {0};
	for(u32 i = 0 ; i < ARRAY_SIZE(inputs); i++) {
		compile_and_generate_shader(&inputs[i], &shaders[i]);
	}

	glslang_finalize_process();


	VkShaderModuleCreateInfo shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.codeSize = shaders[0].size,
		.pCode = shaders[0].data
	};

	buf_fit(vk_ctx->shader_modules, ARRAY_SIZE(inputs));

	for(u32 i = 0; i < ARRAY_SIZE(inputs); i++) {
		CHECK_VK_RESULT(
				vkCreateShaderModule(vk_ctx->device_info.ldevice, &shader_module_create_info, NULL, &vk_ctx->shader_modules[i].shader_module)
				);

		vk_ctx->shader_modules[i].stage = inputs[i].stage;
	}


	VkPipelineShaderStageCreateInfo *pipeline_shaders = NULL;

	for(u32 i = 0; i < buf_cap(vk_ctx->shader_modules); i++) {

		VkPipelineShaderStageCreateInfo pipeline_shader = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = NULL, 
			.flags = 0,
			.stage = CONVERT_GLSLANG_STAGE_TO_VK_STAGE(vk_ctx->shader_modules[i].stage),
			.module = vk_ctx->shader_modules[i].shader_module,
			.pName = "main",
			.pSpecializationInfo = NULL
		};

		buf_push(pipeline_shaders, pipeline_shader);
	}

	VkPipelineVertexInputStateCreateInfo vertex_input = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = 0,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = 0
	};

	VkViewport viewport = {
		.x = 0,
		.y = 0,
		.width = (float)wn_ctx->width,
		.height = (float)wn_ctx->height,
		.minDepth = 0,
		.maxDepth = 1
	};

	VkRect2D scissors = {
		.offset = { .x = 0, .y = 0 },
		.extent = { .width = (u32)viewport.width, .height = (u32)viewport.height }
	};

	VkPipelineViewportStateCreateInfo viewport_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissors
	};

	VkPipelineRasterizationStateCreateInfo rasterization = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = 0,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0

	};

	VkPipelineMultisampleStateCreateInfo multisample_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f,
		.pSampleMask = NULL,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_TRUE
	};


	VkPipelineColorBlendAttachmentState blend_attachment = {
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
						  VK_COLOR_COMPONENT_G_BIT |
						  VK_COLOR_COMPONENT_B_BIT |
						  VK_COLOR_COMPONENT_A_BIT
	};

	VkPipelineColorBlendStateCreateInfo blend_color = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_CLEAR,
		.attachmentCount = 1,
		.pAttachments = &blend_attachment,
		.blendConstants[0] = 0,
		.blendConstants[1] = 0,
		.blendConstants[2] = 0,
		.blendConstants[3] = 0
	};


	VkDynamicState dy_states[] = {
		VK_DYNAMIC_STATE_VIEWPORT, 
		VK_DYNAMIC_STATE_BLEND_CONSTANTS,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
	};

	VkPipelineDynamicStateCreateInfo dynamic_states = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.dynamicStateCount = ARRAY_SIZE(dy_states),
		.pDynamicStates = dy_states
	};


	VkPipelineLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.setLayoutCount = 0,
		.pushConstantRangeCount = 0
	};

	VkPipelineLayout layout;

	CHECK_VK_RESULT(
			vkCreatePipelineLayout(vk_ctx->device_info.ldevice, &layout_info, NULL, &layout)
			);

	VkGraphicsPipelineCreateInfo graphics_pipeline = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stageCount = buf_len(pipeline_shaders),
		.pStages = pipeline_shaders,
		.pVertexInputState = &vertex_input,
		.pInputAssemblyState = &input_assembly,
		.pTessellationState = NULL,
		.pViewportState = &viewport_state,
		.pMultisampleState = &multisample_state,
		.pDepthStencilState = NULL,
		.pColorBlendState = &blend_color,
		.pDynamicState = &dynamic_states,
		.layout = layout,
		.renderPass = 

	};

}

int main(int argc, char **argv) {



	// struct_type_check();

	load_vulkan_funcs();
	sh_vulkan_context_t vk_ctx = setup_vulkan();


	// check_physical_device_groups(&vk_ctx);

	sh_window_context_t *ctx = setup();
	setup_graphics_pipeline(ctx, &vk_ctx);

	setup_surface_and_rendering(ctx, &vk_ctx);
#if 0
	while(!ctx->should_close) {
		sh_handle_events(ctx);

		if(ctx->mouse->left.pressed) {
			printf("%d %d %d %d\n", ctx->mouse->x, ctx->mouse->y, ctx->mouse->left.pressed_once, ctx->mouse->right.pressed_once);
		}
	}
#endif

	destroy_vulkan_instance(&vk_ctx);
	fflush(stdout);

	return 0;
}
