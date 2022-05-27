VKAPI_ATTR VkBool32 VKAPI_CALL debug_dump_everything(
		VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT *data,
		void *user_data) {



	log_debug_gl_trackerl("%s", data->pMessage );

	if(user_data != NULL && message_type == 0 && msg_severity == 0) {
		log_debugl("we have user data");
	}

	return VK_FALSE;
}

VkBool32 VKAPI_PTR create_instance_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT *data,
		void *user_data) {
	log_debugl("%s", data->pMessage );

	if(user_data != NULL && message_type == 0 && msg_severity == 0) {
		log_debugl("we have user data");
	}

	return VK_FALSE;
}

void sh_setup_debug_callbacks(sh_vulkan_context_t *vk_ctx) {

	VkDebugUtilsMessengerCreateInfoEXT debug_ext_create_instance = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity =
			// VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
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

sh_vulkan_context_t* sh_setup_vulkan_instance(sh_vulkan_context_t *vk_ctx, sh_pdev_select_func phy_device_selector) {

	vk_ctx->queue_family_selected = 0xFFFFFFFF;

	u32 v = 0;
	CHECK_VK_RESULT( vkEnumerateInstanceVersion(&v) );
	log_debugl("Vulkan Version: %d.%d.%d", VK_VERSION_MAJOR(v), VK_VERSION_MINOR(v), VK_VERSION_PATCH(v));

    u32 requested_layers_count = SH_ARRAY_SIZE(gl_layers_to_enable);
    u32 requested_inst_ext_count = SH_ARRAY_SIZE(gl_instance_extensions_to_enable);

    u32 enabled_layers_count = 0;
    u32 enabled_ext_count = 0;
	const char **vulkan_layers_enabled = sh_get_enabled_layers(gl_layers_to_enable, requested_layers_count, &enabled_layers_count);
	const char **vulkan_instance_ext   = sh_get_enabled_instance_ext(gl_instance_extensions_to_enable, requested_inst_ext_count, &enabled_ext_count);

	vk_ctx->layers_enabled = vulkan_layers_enabled;
	vk_ctx->extensions_enabled = vulkan_instance_ext;

	// Application info
	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.applicationVersion = VK_MAKE_VERSION(0, 0, 1),
		.pApplicationName = "Vulkan Testing Ground",
		.pEngineName = "sh_it",
		.engineVersion = VK_MAKE_VERSION(1, 3, 0),
		.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0),
	};


	VkValidationFeatureEnableEXT validation_features_to_enable[] = {
		VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
		VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
		VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
	};

	VkValidationFeaturesEXT val_features = {
		.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
		.pNext = NULL,
		.enabledValidationFeatureCount = SH_ARRAY_SIZE(validation_features_to_enable),
		.pEnabledValidationFeatures = validation_features_to_enable,
		.disabledValidationFeatureCount = 0
	};

	VkDebugUtilsMessengerCreateInfoEXT debug_ext_create_instance = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.pNext = &val_features,
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
		.pApplicationInfo = &app_info,
		.enabledLayerCount = enabled_layers_count,
		.ppEnabledLayerNames = vulkan_layers_enabled,
		.enabledExtensionCount = enabled_ext_count,
		.ppEnabledExtensionNames = vulkan_instance_ext
	};

	
	CHECK_VK_RESULT_MSG(
		vkCreateInstance(&create_info, NULL, &vk_ctx->instance),
		"We couldn't create a VkInstance"
	);

	sh_load_vulkan_instance_funcs(&vk_ctx->instance);
	sh_setup_debug_callbacks(vk_ctx);

	// Setup Physical Devices
	sh_select_physical_device(vk_ctx, phy_device_selector);

	if(vk_ctx->device_info.device == VK_NULL_HANDLE) {
		log_debugl("No physical devices availabe that pass our requirement, can't continue vulkan setup.");
		return vk_ctx;
	}

	return vk_ctx;
}

void sh_setup_physical_device_groups(sh_vulkan_context_t *vk_ctx) {
	u32 count = 0;
	CHECK_VK_RESULT( vkEnumeratePhysicalDeviceGroups(vk_ctx->instance, &count, NULL));

	log_debugl("Device Group Count: %d", count);

	VkPhysicalDeviceGroupProperties *device_groups = (VkPhysicalDeviceGroupProperties*)calloc(1, sizeof(VkPhysicalDeviceGroupProperties));

	for(u32 i = 0; i < count; i++) {
		device_groups[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GROUP_PROPERTIES;
	}

	CHECK_VK_RESULT( vkEnumeratePhysicalDeviceGroups(vk_ctx->instance, &count, device_groups));

	for(u32  i = 0; i < count; i++) {
		log_debugl("DG[%d] - Device Count: %d", i,  device_groups[i].physicalDeviceCount);
	}

	// free(device_groups);

}

void sh_setup_surface_and_rendering(sh_window_context_t *wn, sh_vulkan_context_t *vk_ctx) {
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


	VkSurfaceCapabilities2KHR surface_cap = { .sType = VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR };
	VkPhysicalDeviceSurfaceInfo2KHR surface_inf = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, .surface = vk_ctx->surface };

	CHECK_VK_RESULT(
		vkGetPhysicalDeviceSurfaceCapabilities2KHR(
			vk_ctx->device_info.device, &surface_inf, &surface_cap
		)
	);

	sh_print_surface_capabilities(&surface_cap.surfaceCapabilities);

	u32 surface_format_count = 0;

	// Get the counts
	CHECK_VK_RESULT(
			vkGetPhysicalDeviceSurfaceFormats2KHR(vk_ctx->device_info.device, &surface_inf, &surface_format_count, NULL)
			);

	VkSurfaceFormat2KHR *surface_formats = (VkSurfaceFormat2KHR *)calloc(surface_format_count, sizeof(VkSurfaceFormat2KHR));

	for(u32 i = 0; i < surface_format_count; i++) {
		surface_formats[i].sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
	}

	// Get actual numbers
	CHECK_VK_RESULT(
			vkGetPhysicalDeviceSurfaceFormats2KHR(vk_ctx->device_info.device, &surface_inf, &surface_format_count, surface_formats)
			);


	log_debugl("Number for Surface Formats: %d", surface_format_count);
	log_debugl("Surface Formats:");
	for(u32 i = 0; i < surface_format_count; i++) {
		log_debug_morel("\t[%d] %-35s - %s", i,
				sh_get_format_flag_name(surface_formats[i].surfaceFormat.format),
				sh_get_surface_format_color_space_name(surface_formats[i].surfaceFormat.colorSpace)
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
	log_debugl("Surface Present Modes [%d]", surface_present_mode);
	for(u32 i = 0; i < surface_present_mode; i++) {
		log_debug_morel("\t[%d] %s", i, sh_get_present_mode_name(present_modes[i]));
	}

}


void sh_setup_queue_family(sh_vulkan_context_t *vk_ctx, sh_qfamily_select_func qsel_func) {

	sh_select_device_queue_family(vk_ctx, qsel_func);

	if(vk_ctx->queue_family_selected == 0xFFFFFFFF) {
		log_debugl("We couldn't select a queue family that matched out flags");
		return;
	}

	float qpriority = 1.0;
	VkDeviceQueueCreateInfo queue_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.flags = 0,
		.queueFamilyIndex = vk_ctx->queue_family_selected,
		.queueCount = 1,
		.pQueuePriorities = &qpriority
	};

	// // TODO(sharo): its better to pass features someother way? 
    // VkPhysicalDeviceFeatures features_enabled = {
	// 	.fillModeNonSolid = 1,
	// 	.wideLines = 1,
	// 	.largePoints = 1,
	// };

	u32 dev_ext_count = SH_ARRAY_SIZE(gl_physical_device_extensions_to_enable);
	u32 dev_enabled_ext_count = 0;
	vk_ctx->pdevice_extensions_enabled = sh_get_enabled_device_ext(vk_ctx, gl_physical_device_extensions_to_enable, dev_ext_count, &dev_enabled_ext_count);

	VkPhysicalDeviceDescriptorIndexingFeatures partial_bind = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
		.pNext = vk_ctx->device_info.features->pNext,
		.descriptorBindingPartiallyBound = 1,
		.runtimeDescriptorArray = 1
	};

	vk_ctx->device_info.features->pNext = &partial_bind;

	VkDeviceCreateInfo device_create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.flags = 0,
		.pNext = vk_ctx->device_info.features,
		.pQueueCreateInfos = &queue_create_info,
		.queueCreateInfoCount = 1,
		.ppEnabledExtensionNames = vk_ctx->pdevice_extensions_enabled,
		.enabledExtensionCount = dev_enabled_ext_count,
		.pEnabledFeatures = NULL 
	};

	CHECK_VK_RESULT_MSG(
		vkCreateDevice(
			vk_ctx->device_info.device,
			&device_create_info,
			NULL,
			&vk_ctx->device_info.ldevice
		),
		"We couldn't create a logical device"
	);

	// Device Operations
	sh_load_vulkan_device_funcs(&vk_ctx->device_info.ldevice);

	VkDeviceQueueInfo2 device_queue_info =  {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2,
		.pNext = NULL,
		.flags = 0,
		.queueFamilyIndex = vk_ctx->queue_family_selected,
		.queueIndex = 0
	};

	vkGetDeviceQueue2(vk_ctx->device_info.ldevice, &device_queue_info, &vk_ctx->queue);

	log_debugl("Queue handle: %p", vk_ctx->queue);
	if(vk_ctx->queue == VK_NULL_HANDLE) {
		log_debugl("Couldn't create Queue");
	}

}

void sh_setup_swapchain(sh_window_context_t *win_ctx, sh_vulkan_context_t *vk_ctx) {

	VkSwapchainCreateInfoKHR swapchain_create = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = NULL,
		.flags = 0,
		.surface = vk_ctx->surface,
		.minImageCount = 3,
		.imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
		.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		.imageExtent = { .width = win_ctx->width, .height =  win_ctx->height },
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = NULL,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
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

	log_debugl("Image count: %d", image_count);
	for(u32 i = 0; i < image_count; i++) {
		log_debug_morel("\t[%d] %p - %p", i, ims + i, ims[i]);
	}

	buf_fit(vk_ctx->img_views, image_count);
	buf_fit(vk_ctx->img_formats, image_count);

	VkImageViewCreateInfo imv_create_infos = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = VK_FORMAT_B8G8R8A8_SRGB,
	};

	imv_create_infos.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imv_create_infos.subresourceRange.baseMipLevel = 0;
	imv_create_infos.subresourceRange.levelCount = 1;
	imv_create_infos.subresourceRange.baseArrayLayer = 0;
	imv_create_infos.subresourceRange.layerCount = 1;

	for(u32 i = 0; i < image_count; i++) {
		VkImageView img_v;
		imv_create_infos.image = ims[i];
		CHECK_VK_RESULT(vkCreateImageView(vk_ctx->device_info.ldevice, &imv_create_infos, NULL, &img_v));

		buf_push(vk_ctx->img_formats, VK_FORMAT_B8G8R8A8_SRGB);
		buf_push(vk_ctx->img_views, img_v);
	}

	log_debugl("Image Views:");
	for(u32 i = 0; i < buf_len(vk_ctx->img_views); i++) {
		log_debug_morel("\t%p - %p", vk_ctx->img_views + i, vk_ctx->img_views[i]);
	}
}


void sh_destroy_command_pool(sh_vulkan_context_t *vk_ctx) {
	vkDestroyCommandPool(vk_ctx->device_info.ldevice, vk_ctx->cmd_pool, NULL);
}

void sh_destroy_framebuffers(sh_vulkan_context_t *vk_ctx) {
	for(u32 i = 0; i < buf_len(vk_ctx->framebuffers); i++) {
		vkDestroyFramebuffer(vk_ctx->device_info.ldevice, vk_ctx->framebuffers[i], NULL);
	}

	buf_clear(vk_ctx->framebuffers);
}

void sh_destroy_image_view(sh_vulkan_context_t *vk_ctx) {
	for(u32 i = 0; i < buf_len(vk_ctx->img_views); i++) {
		vkDestroyImageView(vk_ctx->device_info.ldevice, vk_ctx->img_views[i], NULL);
	}
	buf_clear(vk_ctx->img_views);
}

void sh_destroy_graphics_pipeline(sh_vulkan_context_t* vk_ctx) {

	vkDestroySwapchainKHR(vk_ctx->device_info.ldevice, vk_ctx->swapchain, NULL);
	vkDestroyPipelineLayout(vk_ctx->device_info.ldevice, vk_ctx->layout, NULL);
	vkDestroyPipeline(vk_ctx->device_info.ldevice, vk_ctx->pipeline, NULL);

}

void sh_destroy_device_memory(sh_vulkan_context_t *vk_ctx) {
	for(u32 i = 0; i < buf_len(vk_ctx->mem->allocs); i++) {
		vkDestroyBuffer(vk_ctx->device_info.ldevice, vk_ctx->mem->allocs[i].buf, NULL);
	}

	for(u32 i = 0; i < buf_len(vk_ctx->mem->image_allocs); i++) {
		vkDestroyImage(vk_ctx->device_info.ldevice, vk_ctx->mem->image_allocs[i].img, NULL);
	}

	for(u32 i = 0; i < buf_len(vk_ctx->mem->image_view_allocs); i++) {
		vkDestroyImageView(vk_ctx->device_info.ldevice, vk_ctx->mem->image_view_allocs[i].handle, NULL);
	}
	
	for(u32 i = 0; i < buf_len(vk_ctx->mem->sampler_allocs); i++) {
		vkDestroySampler(vk_ctx->device_info.ldevice, vk_ctx->mem->sampler_allocs[i].handle, NULL);
	}

	for(u32 i = 0; i < buf_len(vk_ctx->mem->device_mems); i++) {
		vkFreeMemory(vk_ctx->device_info.ldevice, vk_ctx->mem->device_mems[i].mem, NULL);
	}
}

void sh_destroy_descriptors(sh_vulkan_context_t *vk_ctx) {
	for(u32 i = 0; i < buf_len(vk_ctx->descriptor_pools); i++) {
		vkDestroyDescriptorPool(vk_ctx->device_info.ldevice, vk_ctx->descriptor_pools[i], NULL);
	}

	for(u32 i = 0; i < buf_len(vk_ctx->set_layouts); i++) {
		vkDestroyDescriptorSetLayout(vk_ctx->device_info.ldevice, vk_ctx->set_layouts[i], NULL);
	}
}


void sh_destroy_(sh_vulkan_context_t *vk_ctx) {
	for(u32 i = 0; i < buf_len(vk_ctx->descriptor_pools); i++) {
		vkDestroyDescriptorPool(vk_ctx->device_info.ldevice, vk_ctx->descriptor_pools[i], NULL);
	}

	for(u32 i = 0; i < buf_len(vk_ctx->set_layouts); i++) {
		vkDestroyDescriptorSetLayout(vk_ctx->device_info.ldevice, vk_ctx->set_layouts[i], NULL);
	}
}

void destroy_vulkan_instance(sh_vulkan_context_t *vk_ctx) {

	sh_destroy_descriptors(vk_ctx);
	sh_destroy_device_memory(vk_ctx);
	sh_destroy_command_pool(vk_ctx);
	vkDestroySemaphore(vk_ctx->device_info.ldevice, vk_ctx->present_semaphore, NULL);
	vkDestroySemaphore(vk_ctx->device_info.ldevice, vk_ctx->render_semaphore, NULL);
	sh_destroy_framebuffers(vk_ctx);
	
	for(u32 i = 0; i < buf_len(vk_ctx->shader_modules); i++) {
		vkDestroyShaderModule( vk_ctx->device_info.ldevice, vk_ctx->shader_modules[i].shader_module, NULL);
	}

	sh_destroy_image_view(vk_ctx);
	sh_destroy_graphics_pipeline(vk_ctx);

    vkDestroyRenderPass(vk_ctx->device_info.ldevice, vk_ctx->render_pass, NULL);
	vkDestroySurfaceKHR(vk_ctx->instance, vk_ctx->surface, NULL);
	vkDestroyDevice(vk_ctx->device_info.ldevice, NULL);
	vkDestroyDebugUtilsMessengerEXT(vk_ctx->instance, vk_ctx->debug_msgr, NULL);
	vkDestroyInstance(vk_ctx->instance, NULL);
}



void struct_type_check(void) {
#define STRUCT_NAME_SIZE(field, struct) log_debugl("Struct: %s->%s s:%lld", #field, #struct, sizeof(struct))

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

void sh_create_render_pass(sh_vulkan_context_t *vk_ctx) {

	// TODO(sharo): dependency passes might be needed if I don't use "waitQueueIdle"
#if 1

    VkAttachmentDescription2 descriptor_attachments[] = {
		{
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
			.pNext = NULL,
			.flags = 0,
			.format = VK_FORMAT_B8G8R8A8_SRGB,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		},

		{
			.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
			.pNext = NULL,
			.flags = 0,
			.format = VK_FORMAT_D32_SFLOAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		}

	};

    VkAttachmentReference2 attach_ref = {
        .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
        .pNext = NULL,
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

	VkAttachmentReference2 depth_attachment = {
		.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
		.pNext = NULL,
		.attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription2 sub_pass = {
        .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
        .pNext = NULL,
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = NULL,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attach_ref,
		.pDepthStencilAttachment = &depth_attachment
    };


	VkSubpassDependency2 sub_dep = {
		.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
		.pNext = NULL,
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT ,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};

    VkRenderPassCreateInfo2 create_render_p_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
        .pNext = NULL,
        .flags = 0,
        .attachmentCount = SH_ARRAY_SIZE(descriptor_attachments),
        .pAttachments = descriptor_attachments,
        .subpassCount = 1,
        .pSubpasses = &sub_pass,
		.dependencyCount = 1,
		.pDependencies = &sub_dep
    };


    log_debugl("Correlated Mask Count: %d", create_render_p_info.correlatedViewMaskCount);

	CHECK_VK_RESULT(
		vkCreateRenderPass2(vk_ctx->device_info.ldevice, &create_render_p_info, NULL, &vk_ctx->render_pass)
	);

#else

    VkAttachmentDescription descriptor_attachment = {
        .flags = 0,
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference attach_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription sub_pass = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attach_ref
    };

    VkSubpassDependency dep = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };


    VkRenderPassCreateInfo create_render_p_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &descriptor_attachment,
        .subpassCount = 1,
        .pSubpasses = &sub_pass,
        .dependencyCount = 1,
        .pDependencies = &dep
    };


    
    CHECK_VK_RESULT(
            vkCreateRenderPass(vk_ctx->device_info.ldevice, &create_render_p_info, NULL, &vk_ctx->render_pass)
            );

    log_debugl("Render Pass: %p", vk_ctx->render_pass);
#endif

    // return render_pass;
}


void sh_setup_shader_modules(sh_vulkan_context_t *vk_ctx, sh_vk_spirv_shader_t *shaders, i32 shaders_count) {

	VkShaderModuleCreateInfo shader_module_create_info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
	};

	buf_fit(vk_ctx->shader_modules, (u32)shaders_count);

	for(u32 i = 0; i < (u32)shaders_count; i++) {

		shader_module_create_info.codeSize = shaders[i].size;
		shader_module_create_info.pCode = shaders[i].data;
		VkShaderModule shader_module;

		CHECK_VK_RESULT(
				vkCreateShaderModule(
					vk_ctx->device_info.ldevice,
					&shader_module_create_info,
					NULL,
					&shader_module	
					)
				);

		buf_push(vk_ctx->shader_modules, (sh_vk_shader_module_t){
				.shader_module = shader_module,
				.stage = shaders[i].stage
				});
	}


}


void sh_setup_descriptor_sets(sh_vulkan_context_t* vk_ctx) {

	VkDescriptorSetLayoutBinding bindings[] = {
		{
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		},
		{
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 12,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
			.pImmutableSamplers = NULL
		},
		{
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.descriptorCount = 12,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = NULL
		},
	};


	// VkD

	VkDescriptorBindingFlags flags[] = {
		0,
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
	};

	VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.pNext = NULL,
		.bindingCount = SH_ARRAY_SIZE(bindings),
		.pBindingFlags = flags
	};

	VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &binding_flags,
		.flags = 0,
		.bindingCount = SH_ARRAY_SIZE(bindings),
		.pBindings = bindings,
	};


	buf_fit(vk_ctx->set_layouts, 1);

	VkDescriptorSetLayout layout;

	CHECK_VK_RESULT_MSG(
		vkCreateDescriptorSetLayout(
			vk_ctx->device_info.ldevice,
			&descriptor_layout_create_info,
			NULL,
			&layout),
		"Couldn't create Descriptor Layout"
	);

	buf_push(vk_ctx->set_layouts, layout);


	VkDescriptorPoolSize pool_size[] = {
		{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 13 },
		{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 12 },
	};

	VkDescriptorPoolCreateInfo pool_create_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.maxSets = 1,
		.poolSizeCount = SH_ARRAY_SIZE(pool_size),
		.pPoolSizes = pool_size
	};

	buf_fit(vk_ctx->descriptor_pools, 1);
	VkDescriptorPool pool;

	CHECK_VK_RESULT(
		vkCreateDescriptorPool(
			vk_ctx->device_info.ldevice,
			&pool_create_info,
			NULL,
			&pool	
		)
	);

	buf_push(vk_ctx->descriptor_pools, pool);

	VkDescriptorSetAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = NULL,
		.descriptorSetCount = 1,
		.pSetLayouts = &vk_ctx->set_layouts[0],
		.descriptorPool = vk_ctx->descriptor_pools[0]
	};

	buf_fit(vk_ctx->descriptor_sets, 1);

	VkDescriptorSet descriptor_set; 

	CHECK_VK_RESULT(
		vkAllocateDescriptorSets(
			vk_ctx->device_info.ldevice,
			&alloc_info,
			&descriptor_set
		);
	);

	buf_push(vk_ctx->descriptor_sets, descriptor_set);

}

void sh_setup_graphics_pipeline(sh_window_context_t *wn_ctx, sh_vulkan_context_t *vk_ctx) {
	VkPipelineShaderStageCreateInfo *pipeline_shaders = NULL;

	for(u32 i = 0; i < buf_len(vk_ctx->shader_modules); i++) {

		VkPipelineShaderStageCreateInfo pipeline_shader = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = NULL, 
			.flags = 0,
			.stage = vk_ctx->shader_modules[i].stage,
			.module = vk_ctx->shader_modules[i].shader_module,
			.pName = "main",
			.pSpecializationInfo = NULL
		};

		buf_push(pipeline_shaders, pipeline_shader);
	}

	VkVertexInputBindingDescription vertex_input_binding[] = {
		{ .binding = 0, .stride = sizeof(sh_vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX },
	};

	VkVertexInputAttributeDescription vertex_input_attributes[] = {
		{ .binding = 0, .location = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0 },
		{ .binding = 0, .location = 1, .format = VK_FORMAT_R32G32B32A32_SFLOAT, .offset = offsetof(sh_vertex, color) },
		{ .binding = 0, .location = 2, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(sh_vertex, tex) },
		{ .binding = 0, .location = 3, .format = VK_FORMAT_R32_SINT, .offset = offsetof(sh_vertex, material_id) },
		{ .binding = 0, .location = 4, .format = VK_FORMAT_R32_SINT, .offset = offsetof(sh_vertex, enable_material) }
	};

	VkPipelineVertexInputStateCreateInfo vertex_input = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = NULL,
        .vertexBindingDescriptionCount = SH_ARRAY_SIZE(vertex_input_binding),
		.pVertexBindingDescriptions = vertex_input_binding,
        .vertexAttributeDescriptionCount = SH_ARRAY_SIZE(vertex_input_attributes),
		.pVertexAttributeDescriptions = vertex_input_attributes
	};

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkViewport viewport = {
		.x = 0,
		.y = 0,
		.width = (float)wn_ctx->width,
		.height = (float)wn_ctx->height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
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
		.cullMode = VK_CULL_MODE_FRONT_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0f

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
		.alphaToOneEnable = VK_FALSE
	};

	VkPipelineColorBlendAttachmentState blend_attachment = {
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
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
		.logicOpEnable = VK_TRUE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &blend_attachment,
		.blendConstants[0] = 0,
		.blendConstants[1] = 0,
		.blendConstants[2] = 0,
		.blendConstants[3] = 0
	};

	VkDynamicState dy_states[] = {
		// VK_DYNAMIC_STATE_VIEWPORT, 
		VK_DYNAMIC_STATE_BLEND_CONSTANTS,
		// VK_DYNAMIC_STATE_SCISSOR,
		// VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
	};

	VkPipelineDynamicStateCreateInfo dynamic_states = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.dynamicStateCount = SH_ARRAY_SIZE(dy_states),
		.pDynamicStates = dy_states
	};

	VkPipelineDepthStencilStateCreateInfo depth_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = NULL,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 0.0f
	};

	VkPipelineLayoutCreateInfo layout_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.setLayoutCount = buf_len(vk_ctx->set_layouts),
		.pSetLayouts = vk_ctx->set_layouts,
		.pushConstantRangeCount = 0
	};

	CHECK_VK_RESULT( vkCreatePipelineLayout(vk_ctx->device_info.ldevice, &layout_info, NULL, &vk_ctx->layout) );

	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.stageCount = buf_len(pipeline_shaders),
		.pStages = pipeline_shaders,
        .pRasterizationState = &rasterization,
		.pVertexInputState = &vertex_input,
		.pInputAssemblyState = &input_assembly,
		.pTessellationState = NULL,
		.pViewportState = &viewport_state,
		.pMultisampleState = &multisample_state,
		.pDepthStencilState = &depth_state,
		.pColorBlendState = &blend_color,
		.pDynamicState = &dynamic_states,
		.layout = vk_ctx->layout,
		.renderPass = vk_ctx->render_pass,
        .subpass = 0
	};

    CHECK_VK_RESULT(vkCreateGraphicsPipelines(vk_ctx->device_info.ldevice, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, NULL, &vk_ctx->pipeline));

}

void sh_setup_framebuffers(sh_window_context_t *win_ctx, sh_vulkan_context_t *vk_ctx) {

	sh_vk_image_allocation_t depth_buffer = sh_allocate_2D_image(
		vk_ctx,
		win_ctx->width, win_ctx->height,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_FORMAT_D32_SFLOAT,
		0);

	sh_vk_image_view_allocation_t depth_buffer_view = sh_allocate_image_view(vk_ctx, &depth_buffer,VK_IMAGE_ASPECT_DEPTH_BIT);

	VkImageView views[] = {
		vk_ctx->img_views[0],
		depth_buffer_view.handle
	};

    VkFramebufferCreateInfo frame_buffer = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = 0,
        .flags = 0,
        .renderPass = vk_ctx->render_pass,
        .attachmentCount = SH_ARRAY_SIZE(views),
        .pAttachments = views,
        .width = win_ctx->width,
        .height = win_ctx->height,
        .layers = 1
    };

    for(u32 i = 0; i < buf_len(vk_ctx->img_views); i++) {
        VkFramebuffer fb;
		views[0] = vk_ctx->img_views[i];
        frame_buffer.pAttachments = views;
        CHECK_VK_RESULT(vkCreateFramebuffer(vk_ctx->device_info.ldevice, &frame_buffer, NULL, &fb));
        buf_push(vk_ctx->framebuffers, fb);
    }

}

void sh_setup_cmd_pools_and_buffers(sh_vulkan_context_t *vk_ctx) {
	VkCommandPoolCreateInfo cmd_pool_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,//VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = vk_ctx->queue_family_selected
	};

	CHECK_VK_RESULT(vkCreateCommandPool(vk_ctx->device_info.ldevice, &cmd_pool_info, NULL, &vk_ctx->cmd_pool));

	buf_fit(vk_ctx->cmd_buffers, buf_len(vk_ctx->framebuffers));

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = NULL,
        .commandPool = vk_ctx->cmd_pool,
        .commandBufferCount = buf_len(vk_ctx->framebuffers),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    };

    CHECK_VK_RESULT(vkAllocateCommandBuffers(vk_ctx->device_info.ldevice, &alloc_info, vk_ctx->cmd_buffers));

	for(u32 i = 0; i < buf_len(vk_ctx->framebuffers); i++) {
		buf_push(vk_ctx->cmd_buffers, vk_ctx->cmd_buffers[i]);
	}

	alloc_info.commandBufferCount = 1;
    CHECK_VK_RESULT(vkAllocateCommandBuffers(vk_ctx->device_info.ldevice, &alloc_info, &vk_ctx->temp_buffer));
}

void sh_begin_render(VkCommandBuffer buffer) {
	VkCommandBufferBeginInfo begin_render = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = 0
	};
	CHECK_VK_RESULT_MSG(vkBeginCommandBuffer(buffer, &begin_render), "Couldn't begin buffer recording");
}


void sh_begin_render_pass(sh_window_context_t *win_ctx, sh_vulkan_context_t *vk_ctx, VkCommandBuffer cmd_buffer, VkFramebuffer frame_buffer) {

	VkSubpassBeginInfo subpass_begin = {
		.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO,
		.pNext = NULL,
		.contents = VK_SUBPASS_CONTENTS_INLINE
	};

	VkClearValue clear_vals[] = {
		{ .color = { vk_ctx->clear_color[0], vk_ctx->clear_color[1], vk_ctx->clear_color[2], vk_ctx->clear_color[3], } },
		{ .depthStencil = { .depth = 1.0f } }
	};

	VkRenderPassBeginInfo begin_render_pass = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderPass = vk_ctx->render_pass,
		.framebuffer = frame_buffer,
		.renderArea = {
			.offset = {0, 0},
			.extent = {.width = win_ctx->width, .height = win_ctx->height}
		},
		.clearValueCount = SH_ARRAY_SIZE(clear_vals),
		.pClearValues = clear_vals
	};

	vkCmdBeginRenderPass2(cmd_buffer, &begin_render_pass, &subpass_begin);
}


void sh_end_render_pass(VkCommandBuffer cmd_buffer) {
	VkSubpassEndInfo end_render_pass = {
		.sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO,
		.pNext = NULL
	};
	vkCmdEndRenderPass2(cmd_buffer, &end_render_pass );
}


void sh_setup_cmd_render(sh_window_context_t *win_ctx, sh_vulkan_context_t *vk_ctx) {
    for(u32 i = 0; i < 3; i++) {

		sh_begin_render(vk_ctx->cmd_buffers[i]);
		sh_begin_render_pass(win_ctx, vk_ctx, vk_ctx->cmd_buffers[i], vk_ctx->framebuffers[i]);

        vkCmdBindPipeline(vk_ctx->cmd_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vk_ctx->pipeline);
        vkCmdDraw(vk_ctx->cmd_buffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(vk_ctx->cmd_buffers[i]);

        CHECK_VK_RESULT_MSG(vkEndCommandBuffer(vk_ctx->cmd_buffers[i]), "Failed to end command buffer");
    }

}

void sh_setup_semaphores(sh_vulkan_context_t *vk_ctx) {

    VkSemaphoreCreateInfo semp = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0
    };


    CHECK_VK_RESULT(vkCreateSemaphore(vk_ctx->device_info.ldevice, &semp, NULL, &vk_ctx->render_semaphore));
    CHECK_VK_RESULT(vkCreateSemaphore(vk_ctx->device_info.ldevice, &semp, NULL, &vk_ctx->present_semaphore));
}


void sh_recreate_rendering_resources(sh_window_context_t* ctx, sh_vulkan_context_t *vk_ctx) {

	vkQueueWaitIdle(vk_ctx->queue);

	sh_destroy_framebuffers(vk_ctx);
	sh_destroy_image_view(vk_ctx);
	sh_destroy_graphics_pipeline(vk_ctx);

	sh_setup_graphics_pipeline(ctx, vk_ctx);
	sh_setup_swapchain(ctx, vk_ctx);
	sh_setup_framebuffers(ctx, vk_ctx);
}

void sh_setup_device_memory(sh_vulkan_context_t *vk_ctx) {
	SH_ASSERT_EXIT(vk_ctx != NULL && vk_ctx->instance != VK_NULL_HANDLE, "vulkan context is null");
	vk_ctx->mem = (sh_vk_memory_manager_t*)calloc(1, sizeof(sh_vk_memory_manager_t));
}

sh_window_context_t* sh_win_setup(void) {
	sh_window_context_t *ctx = (sh_window_context_t*)calloc(1, sizeof(sh_window_context_t));

	ctx->width = 1280;
	ctx->height = 720;
	ctx->x = 0;
	ctx->y = 0;
	ctx->window_name = "hello world";

	sh_create_window_win32(ctx);

	return ctx;
}


sh_vulkan_context_t* sh_vk_setup(sh_window_context_t *win_ctx, sh_pdev_select_func physical_device_selector, sh_qfamily_select_func queue_family_selector, sh_vk_spirv_shader_t* spirv_shaders) {

	sh_vulkan_context_t *vk_ctx = (sh_vulkan_context_t*)calloc(1, sizeof(sh_vulkan_context_t));

	sh_setup_vulkan_instance(vk_ctx, physical_device_selector);
	sh_setup_surface_and_rendering(win_ctx, vk_ctx);
	sh_setup_queue_family(vk_ctx, queue_family_selector);

	// needed for recreation
	sh_create_render_pass(vk_ctx);
	sh_setup_shader_modules(vk_ctx, spirv_shaders, buf_len(spirv_shaders));

	sh_setup_descriptor_sets(vk_ctx);
	sh_setup_graphics_pipeline(win_ctx, vk_ctx);
	sh_setup_device_memory(vk_ctx);
	sh_setup_swapchain(win_ctx, vk_ctx);
	sh_setup_framebuffers(win_ctx, vk_ctx);
	sh_setup_cmd_pools_and_buffers(vk_ctx);
	sh_setup_semaphores(vk_ctx);

	return vk_ctx;
}
