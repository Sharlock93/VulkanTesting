#include "../headers/sh_load_vulkan.h"

void sh_load_vulkan_funcs(void) {

#define SH_VK_FUNC(name) name = (PFN_##name)vkGetInstanceProcAddr(NULL, #name);\
	if(name == NULL) { log_debugl("Coudln't load vulkan global function: %s. [%s - %d]", #name, __FILE__, __LINE__); exit(1); }

#if LOAD_VK

	HMODULE vulkan_lib = LoadLibrary("vulkan-1.dll");

	SH_LOAD_VK_GET_INST_PROC_ADDR(vkGetInstanceProcAddr);
	SH_LOAD_VK_GET_INST_PROC_ADDR(vkGetDeviceProcAddr);


	SH_VK_FUNC(vkCreateInstance);
	SH_VK_FUNC(vkEnumerateInstanceExtensionProperties);
	SH_VK_FUNC(vkEnumerateInstanceLayerProperties);
	SH_VK_FUNC(vkEnumerateInstanceVersion);

	// loadglobalfuncsend

#endif

#undef SH_VK_FUNC
}


void sh_load_vulkan_instance_funcs(VkInstance *inst) {

#define SH_VK_FUNC(name) name = (PFN_##name)vkGetInstanceProcAddr(*(inst), #name);\
	if(name == NULL) { log_debugl("Coudln't load vulkan instance function: %s. [%s - %d]", #name, __FILE__, __LINE__); exit(1); }

#if LOAD_VK


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
	SH_VK_FUNC(vkGetPhysicalDeviceImageFormatProperties2);

	SH_VK_FUNC(vkEnumeratePhysicalDeviceGroups);

    
    SH_VK_FUNC(vkCreateFramebuffer);
    SH_VK_FUNC(vkDestroyFramebuffer);
    

    // loadinstancefuncsend
#endif

#undef SH_VK_FUNC

}

void sh_load_vulkan_device_funcs(VkDevice *dev) {

#define SH_VK_FUNC(name) name = (PFN_##name)vkGetDeviceProcAddr(*(dev), #name);\
		if(name == NULL) { log_debugl("Coudln't load vulkan device function: %s. [%s - %d]", #name, __FILE__, __LINE__); exit(1); }

#if LOAD_VK

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
    SH_VK_FUNC(vkDestroyPipelineLayout);

    SH_VK_FUNC(vkCreateGraphicsPipelines);
    SH_VK_FUNC(vkDestroyPipeline);

    SH_VK_FUNC(vkCreateCommandPool);
    SH_VK_FUNC(vkResetCommandPool);
    SH_VK_FUNC(vkDestroyCommandPool);

    SH_VK_FUNC(vkAllocateCommandBuffers);

    SH_VK_FUNC(vkBeginCommandBuffer);
    SH_VK_FUNC(vkCreateSemaphore);
    SH_VK_FUNC(vkDestroySemaphore);

    SH_VK_FUNC(vkAcquireNextImageKHR);

    SH_VK_FUNC(vkQueueSubmit);
    SH_VK_FUNC(vkQueueSubmit2);

	SH_VK_FUNC(vkCreateRenderPass);
	SH_VK_FUNC(vkCreateRenderPass2);
	SH_VK_FUNC(vkDestroyRenderPass);

	SH_VK_FUNC(vkCmdBeginRenderPass2);
    SH_VK_FUNC(vkCmdEndRenderPass2);

	SH_VK_FUNC(vkCmdBeginRenderPass);
	SH_VK_FUNC(vkCmdBindPipeline);
	SH_VK_FUNC(vkCmdDraw);
	SH_VK_FUNC(vkCmdDrawIndexed);
	SH_VK_FUNC(vkCmdEndRenderPass);
	SH_VK_FUNC(vkCmdBlitImage2);
	SH_VK_FUNC(vkCmdBindVertexBuffers);
	SH_VK_FUNC(vkCmdBindIndexBuffer);
	SH_VK_FUNC(vkCmdBindVertexBuffers2);
	SH_VK_FUNC(vkEndCommandBuffer);

	SH_VK_FUNC(vkQueuePresentKHR);
	SH_VK_FUNC(vkQueueWaitIdle);

	SH_VK_FUNC(vkCreateBuffer);
	SH_VK_FUNC(vkDestroyBuffer);
	SH_VK_FUNC(vkGetBufferMemoryRequirements2);
	SH_VK_FUNC(vkGetImageMemoryRequirements2);
	SH_VK_FUNC(vkCmdUpdateBuffer);

	SH_VK_FUNC(vkAllocateMemory);
	SH_VK_FUNC(vkFreeMemory);
	SH_VK_FUNC(vkBindBufferMemory2);
	SH_VK_FUNC(vkBindImageMemory2);
	SH_VK_FUNC(vkMapMemory);
	SH_VK_FUNC(vkUnmapMemory);
	SH_VK_FUNC(vkFlushMappedMemoryRanges);
	SH_VK_FUNC(vkInvalidateMappedMemoryRanges);

	SH_VK_FUNC(vkCmdPipelineBarrier2);

	SH_VK_FUNC(vkCreateDescriptorSetLayout);
	SH_VK_FUNC(vkDestroyDescriptorSetLayout);
	SH_VK_FUNC(vkCreateDescriptorPool);
	SH_VK_FUNC(vkDestroyDescriptorPool);

	SH_VK_FUNC(vkAllocateDescriptorSets);
	SH_VK_FUNC(vkFreeDescriptorSets);
	SH_VK_FUNC(vkUpdateDescriptorSets);
	SH_VK_FUNC(vkCmdBindDescriptorSets);

	SH_VK_FUNC(vkCreateImage);
	SH_VK_FUNC(vkCmdCopyBufferToImage2);
	SH_VK_FUNC(vkCmdCopyBuffer2);
	SH_VK_FUNC(vkDestroyImage);

	SH_VK_FUNC(vkCreateSampler);
	SH_VK_FUNC(vkDestroySampler);

    // loaddevicefuncsend

#endif
#undef SH_VK_FUNC

}

const char** sh_get_enabled_layers(const char **requested_layers, u32 requested_layers_count, u32 *enabled_layers_count) {

#if ENABLE_LAYERS
    if(requested_layers_count == 1 && requested_layers[0] == NULL) { return NULL;}

	u32 c = 0;
	CHECK_VK_RESULT(vkEnumerateInstanceLayerProperties(&c, NULL));
	VkLayerProperties *p = (VkLayerProperties*) malloc(sizeof(VkLayerProperties)*c);
	vkEnumerateInstanceLayerProperties(&c, p);

    const char** names = (const char**)malloc(sizeof(const char **)*requested_layers_count);
    i8 found = 0;
    i8 wrote = 1;
    *enabled_layers_count = 0;
    for(u32 i = 0; i < requested_layers_count; i++) {
        found = 0;
        for(u32 j = 0; j < c; j++) {
            if(strcmp(requested_layers[i], p[j].layerName) == 0) {
                names[*enabled_layers_count] = requested_layers[i];
                found = 1;
                (*enabled_layers_count)++;
                break;
			}
		}
        if(found == 0) {
            if(wrote == 1) {
                puts("These layers where not enabled");
                wrote = 0;
            }
            putchar('\t');
            puts(requested_layers[i]);
        }
	}

	free(p);
	return names;

#else
    return NULL;
#endif

}


const char** sh_get_enabled_instance_ext(const char** requested_extensions, u32 requested_ext_count, u32 *enabled_ext) {

#if ENABLE_EXTENSIONS

	if( requested_ext_count == 1 && requested_extensions[0] == NULL) {
        return NULL;
    }

	u32 c = 0;
	CHECK_VK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &c, NULL));
	VkExtensionProperties *p = (VkExtensionProperties*) malloc(sizeof(VkExtensionProperties)*c);
	CHECK_VK_RESULT(vkEnumerateInstanceExtensionProperties(NULL, &c, p));

    const char** names = (const char**)malloc(sizeof(const char **)*requested_ext_count);
    i8 found = 0;
    i8 wrote = 1;
	for(u32 i = 0; i < requested_ext_count; i++) {
		for(u32 j = 0; j < c; j++) {
			if(strcmp(requested_extensions[i], p[j].extensionName) == 0) {
				names[*enabled_ext] = requested_extensions[i];
                (*enabled_ext)++;
                found = 1;
                break;
			}
		}

        if(found == 0) {
            if(wrote == 1) {
                puts("These extensions were not found");
                wrote = 0;
            }

            putchar('\t');
            puts(requested_extensions[i]);
        }
    }

	free(p);

	return names;
#else
    *enabled_ext = 0;
    return NULL;
#endif

}

const char** sh_get_enabled_device_ext(sh_vulkan_context_t *vk_ctx, const char** requested_extensions, u32 requested_ext_count, u32 *enabled_ext) {
	
#if ENABLE_PDEVICE_EXTENSIONS

    if( requested_ext_count == 1 && requested_extensions[0] == NULL) {
        return NULL;
    }

	u32 c = 0;
	CHECK_VK_RESULT( vkEnumerateDeviceExtensionProperties(vk_ctx->device_info.device, NULL, &c, NULL) );
	VkExtensionProperties *p = (VkExtensionProperties*) malloc(sizeof(VkExtensionProperties)*c);
	CHECK_VK_RESULT(vkEnumerateDeviceExtensionProperties(vk_ctx->device_info.device, NULL, &c, p));


    const char** names = (const char**)malloc(sizeof(const char **)*requested_ext_count);
    i8 found = 0;
    i8 wrote = 1;
	for(u32 i = 0; i < requested_ext_count; i++) {
		for(u32 j = 0; j < c; j++) {
			if(strcmp(requested_extensions[i], p[j].extensionName) == 0) {
				names[*enabled_ext] = requested_extensions[i];
                (*enabled_ext)++;
                found = 1;
                break;
			}
		}

        if(found == 0) {
            if(wrote == 1) {
                puts("These device extensions were not found");
                wrote = 0;
            }

            putchar('\t');
            puts(requested_extensions[i]);
        }
    }

	free(p);

	return names;
#else
    *enabled_ext = 0;
    return NULL;
#endif

}

void sh_select_physical_device(sh_vulkan_context_t *vk_ctx, sh_pdev_select_func select_function) {

	u32 count = 0;
	CHECK_VK_RESULT_MSG(
        vkEnumeratePhysicalDevices(vk_ctx->instance, &count, NULL),
        "Couldn't Enumerate devices"
	);

	log_debugl("%d Devices Found", count);
	
	VkPhysicalDevice *devices = NULL;
	devices = (VkPhysicalDevice*)malloc(sizeof(devices)*count);
	CHECK_VK_RESULT_MSG(
        vkEnumeratePhysicalDevices( vk_ctx->instance, &count, devices),
        "Couldn't get physical devices"
	);

	VkPhysicalDeviceProperties2 p = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
	};
	
	VkPhysicalDevice selected_device = VK_NULL_HANDLE;

	//TODO(sharo): could use a better approach to select a device
	for(i32 i = 0; i < (i32)count; i++) {
		vkGetPhysicalDeviceProperties2(devices[i], &p);
		putchar('\t');
		sh_print_pdevice_info(&p);
        if(select_function(p.properties)) {
            selected_device = devices[i];
        }
    }

	vk_ctx->device_info.features = (VkPhysicalDeviceFeatures2*)calloc(1, sizeof(VkPhysicalDeviceFeatures2));
	VkPhysicalDeviceVulkan13Features *vk13_feat = (VkPhysicalDeviceVulkan13Features*)calloc(1, sizeof(VkPhysicalDeviceVulkan13Features));

	vk_ctx->device_info.features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	vk13_feat->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;

	vk_ctx->device_info.features->pNext = vk13_feat; 


	if(selected_device != VK_NULL_HANDLE) {
		log_debugl("Selected Device:");
		vk_ctx->device_info.device = selected_device;
		vkGetPhysicalDeviceProperties2(vk_ctx->device_info.device, &p);
		sh_print_pdevice_info(&p);

		vkGetPhysicalDeviceFeatures2(vk_ctx->device_info.device, vk_ctx->device_info.features);
		sh_print_pdevice_features(vk_ctx->device_info.features);

		VkPhysicalDeviceMemoryProperties2 device_mem = {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2
		};

		vkGetPhysicalDeviceMemoryProperties2(selected_device, &device_mem);

		buf_fit(vk_ctx->device_info.mem_types, device_mem.memoryProperties.memoryTypeCount);

		for(u32 i = 0; i < device_mem.memoryProperties.memoryTypeCount; i++ ) {
			buf_push(vk_ctx->device_info.mem_types, device_mem.memoryProperties.memoryTypes[i].propertyFlags);
		}

		sh_print_pdevice_memory_properties(&device_mem.memoryProperties);
	}

	free(devices);
}



void sh_select_device_queue_family(sh_vulkan_context_t *vk_ctx, sh_qfamily_select_func select_function) {
	
	sh_vulkan_pdevice *pdev = &vk_ctx->device_info;

	// TODO(sharo): this might be so common in Vulkan that we can write a macro for? 

	u32 count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2(pdev->device, &count, NULL);
	VkQueueFamilyProperties2 *queue_properties = (VkQueueFamilyProperties2*) malloc(count*sizeof(VkQueueFamilyProperties2));

	for(u32 i = 0; i < count; i++) {
		queue_properties[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
		queue_properties[i].pNext = NULL;
	}

	vkGetPhysicalDeviceQueueFamilyProperties2(pdev->device, &count, queue_properties);

	i32 selected_queue_family_index = -1;

	for(u32 i = 0; i < count; i++) {
		log_debugl("Queue Family Index [%d]", i);
		log_debug_morel("\tQueue Count: %d", queue_properties[i].queueFamilyProperties.queueCount);

		VkQueueFlagBits flags = queue_properties[i].queueFamilyProperties.queueFlags;
		sh_print_queue_family_flags(flags);

		if(select_function(vk_ctx, queue_properties + i, i)) {
			selected_queue_family_index = i;
			break;
		}
	}

	if(selected_queue_family_index != -1) {
		log_debugl("We have selected the queue family index [%d]", selected_queue_family_index);
		u32 qcount =  queue_properties[selected_queue_family_index].queueFamilyProperties.queueCount;
		VkQueueFlagBits flags = queue_properties[selected_queue_family_index].queueFamilyProperties.queueFlags;
		log_debugl("\tQueue Count: %d", qcount);
		sh_print_queue_family_flags(flags);
		vk_ctx->queue_family_selected = selected_queue_family_index;
	} else {
	}

}

inline VkShaderStageFlagBits sh_shader_type_to_vk_stage(sh_shader_type_t type) {
	return _sh_type_to_stage_map[type];
}

sh_vk_spirv_shader_t sh_compile_shader_input(sh_shader_input_t *shader) {

	glslang_initialize_process();

	char* shader_source = sh_read_file(shader->filename, NULL);

	if(shader_source == NULL) {
		log_debugl("Shader source for file %s couldn't be read", shader->filename);
		exit(-1);
	}
	
	glslang_stage_t stage;
	switch(shader->type) {
		case SH_SHADER_TYPE_VERTEX:
			stage = GLSLANG_STAGE_VERTEX;
			break;
		case SH_SHADER_TYPE_FRAGMENT:
			stage = GLSLANG_STAGE_FRAGMENT;
			break;
		default:
			SH_ASSERT_EXIT(false, "Shader Stage invalid");
			exit(1);
	}

	const glslang_input_t input = {
		.language = GLSLANG_SOURCE_GLSL,
		.stage = stage,
		.client = GLSLANG_CLIENT_VULKAN,
		.client_version = GLSLANG_TARGET_VULKAN_1_3,
		.target_language = GLSLANG_TARGET_SPV,
		.target_language_version = GLSLANG_TARGET_SPV_1_3,
		.code = shader_source,
		.default_version = 450,
		.default_profile = GLSLANG_CORE_PROFILE,
		.force_default_version_and_profile = 0,
		.forward_compatible = 0,
		.messages = GLSLANG_MSG_DEFAULT_BIT|GLSLANG_MSG_DEBUG_INFO_BIT|GLSLANG_MSG_SPV_RULES_BIT|GLSLANG_MSG_ENHANCED,
		.resource = &sh_vk_default_resource_limits
	};

	glslang_program_t *program = glslang_program_create();
	glslang_shader_t *shad = glslang_shader_create(&input);

	if(!glslang_shader_preprocess(shad, &input)) {
		log_debugl("Couldn't do shader preprocessing..");
		log_debugl("%s", glslang_shader_get_info_log(shad));
	}

	if(!glslang_shader_parse(shad, &input)) {
		log_debugl("Coudln't parse shader: %s", shader->filename);
		log_debugl("%s", glslang_shader_get_info_log(shad));
	}

	glslang_program_add_shader(program, shad);

	if(!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
		log_debugl("For shader (%s) coudln't link the program.", shader->filename);
		log_debug_morel("Linker Error:\n%s", glslang_program_get_info_log(program));
	}

	glslang_program_SPIRV_generate(program, input.stage);

	if(glslang_program_SPIRV_get_messages(program)) {
		log_debugl("generating spir-v failed: \n%s\n%s",
				glslang_program_SPIRV_get_messages(program),
				glslang_program_get_info_debug_log(program)
		);
	}

	sh_vk_spirv_shader_t compiled_shader = {0};

	size_t data_size_in_bytes = glslang_program_SPIRV_get_size(program)*sizeof(u32);
	compiled_shader.data = (u32*)calloc(data_size_in_bytes, sizeof(char));
	memcpy(compiled_shader.data, glslang_program_SPIRV_get_ptr(program), data_size_in_bytes);
	compiled_shader.size = data_size_in_bytes;
	compiled_shader.stage = sh_shader_type_to_vk_stage(shader->type);

	glslang_program_delete(program);

	glslang_finalize_process();
	free(shader_source);
	return compiled_shader;
}

