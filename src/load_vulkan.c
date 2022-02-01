// #include <libloaderapi.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>


char *VK_DEVICE_TYPE_NAMES[] = {
	[VK_PHYSICAL_DEVICE_TYPE_OTHER] = "Other",
	[VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU] = "iGPU",
	[VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU] = "dGPU",
	[VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU] = "vGPU",
	[VK_PHYSICAL_DEVICE_TYPE_CPU] = "CPU",
};


#define ADD_PHYSICAL_DEVICE_FEATURE_NAME(name) [offsetof(VkPhysicalDeviceFeatures, name)/4] = #name

char *VK_DEVICE_FEATURE_NAMES[] = {
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(robustBufferAccess),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(fullDrawIndexUint32),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(imageCubeArray),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(independentBlend),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(geometryShader),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(tessellationShader),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(sampleRateShading),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(dualSrcBlend),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(logicOp),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(multiDrawIndirect),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(drawIndirectFirstInstance),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(depthClamp),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(depthBiasClamp),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(fillModeNonSolid),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(depthBounds),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(wideLines),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(largePoints),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(alphaToOne),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(multiViewport),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(samplerAnisotropy),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(textureCompressionETC2),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(textureCompressionASTC_LDR),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(textureCompressionBC),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(occlusionQueryPrecise),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(pipelineStatisticsQuery),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(vertexPipelineStoresAndAtomics),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(fragmentStoresAndAtomics),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderTessellationAndGeometryPointSize),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderImageGatherExtended),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderStorageImageExtendedFormats),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderStorageImageMultisample),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderStorageImageReadWithoutFormat),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderStorageImageWriteWithoutFormat),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderUniformBufferArrayDynamicIndexing),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderSampledImageArrayDynamicIndexing),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderStorageBufferArrayDynamicIndexing),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderStorageImageArrayDynamicIndexing),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderClipDistance),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderCullDistance),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderFloat64),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderInt64),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderInt16),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderResourceResidency),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(shaderResourceMinLod),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(sparseBinding),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(sparseResidencyBuffer),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(sparseResidencyImage2D),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(sparseResidencyImage3D),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(sparseResidency2Samples),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(sparseResidency4Samples),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(sparseResidency8Samples),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(sparseResidency16Samples),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(sparseResidencyAliased),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(variableMultisampleRate),
    ADD_PHYSICAL_DEVICE_FEATURE_NAME(inheritedQueries),
};

#undef ADD_PHYSICAL_DEVICE_FEATURE_NAME


#define ADD_QUEUE_FAMILY_FLAG(name) [name] = #name


const char* const VK_QUEUE_FAMILY_FLAG_NAMES[] = {

	ADD_QUEUE_FAMILY_FLAG(VK_QUEUE_GRAPHICS_BIT),
	ADD_QUEUE_FAMILY_FLAG(VK_QUEUE_COMPUTE_BIT),
	ADD_QUEUE_FAMILY_FLAG(VK_QUEUE_TRANSFER_BIT),
	ADD_QUEUE_FAMILY_FLAG(VK_QUEUE_SPARSE_BINDING_BIT),
	ADD_QUEUE_FAMILY_FLAG(VK_QUEUE_PROTECTED_BIT),
#ifdef VK_ENABLE_BETA_EXTENSIONS
	ADD_QUEUE_FAMILY_FLAG(VK_QUEUE_VIDEO_DECODE_BIT_KHR),
#endif
#ifdef VK_ENABLE_BETA_EXTENSIONS
	ADD_QUEUE_FAMILY_FLAG(VK_QUEUE_VIDEO_ENCODE_BIT_KHR),
#endif

};


#define GET_TYPE_NAME(type) VK_DEVICE_TYPE_NAMES[type]
#define GET_FEATURE_NAME(feature) VK_DEVICE_FEATURE_NAMES[feature]
#define GET_QUEUE_FLAG_NAME(flag) VK_QUEUE_FAMILY_FLAG_NAMES[flag]

#define SH_VK_FUNC_NAME(name) PFN_##name name
#define SH_LOAD_VK_GET_INST_PROC_ADDR(name) name = (PFN_##name)GetProcAddress(vulkan_lib, #name)
#define SH_FUNC_LOAD_GL(name) name = (PFN_##name)vkGetInstanceProcAddr(NULL, #name)
#define SH_FUNC_LOAD_IN(name) name = (PFN_##name)vkGetInstanceProcAddr(*(inst), #name)
#define SH_FUNC_LOAD_DEV(name) name = (PFN_##name)vkGetDeviceProcAddr(*(dev), #name)
										   



SH_VK_FUNC_NAME(vkGetInstanceProcAddr);
SH_VK_FUNC_NAME(vkGetDeviceProcAddr);

SH_VK_FUNC_NAME(vkEnumerateInstanceVersion);
SH_VK_FUNC_NAME(vkCreateInstance);
SH_VK_FUNC_NAME(vkEnumerateInstanceExtensionProperties);
SH_VK_FUNC_NAME(vkEnumerateDeviceExtensionProperties);
SH_VK_FUNC_NAME(vkEnumerateInstanceLayerProperties);
SH_VK_FUNC_NAME(vkGetPhysicalDeviceProperties);
SH_VK_FUNC_NAME(vkEnumeratePhysicalDevices);
SH_VK_FUNC_NAME(vkCreateDevice);

SH_VK_FUNC_NAME(vkGetPhysicalDeviceFeatures);
SH_VK_FUNC_NAME(vkGetPhysicalDeviceQueueFamilyProperties);
SH_VK_FUNC_NAME(vkGetDeviceQueue);


void load_vulkan_funcs(void) {
	HMODULE vulkan_lib = LoadLibrary("vulkan-1.dll");

	SH_LOAD_VK_GET_INST_PROC_ADDR(vkGetInstanceProcAddr);
	SH_LOAD_VK_GET_INST_PROC_ADDR(vkGetDeviceProcAddr);

	SH_FUNC_LOAD_GL(vkCreateInstance);
	SH_FUNC_LOAD_GL(vkEnumerateInstanceExtensionProperties);
	SH_FUNC_LOAD_GL(vkEnumerateInstanceLayerProperties);
	SH_FUNC_LOAD_GL(vkEnumerateInstanceVersion);

	assert(vkCreateInstance != NULL);
	
}


void load_vulkan_instance_funcs(VkInstance *inst) {
	SH_FUNC_LOAD_IN(vkEnumeratePhysicalDevices);
	SH_FUNC_LOAD_IN(vkGetPhysicalDeviceProperties);
	SH_FUNC_LOAD_IN(vkGetPhysicalDeviceFeatures);
	SH_FUNC_LOAD_IN(vkGetPhysicalDeviceQueueFamilyProperties);
	SH_FUNC_LOAD_IN(vkCreateDevice);
	SH_FUNC_LOAD_IN(vkEnumerateDeviceExtensionProperties);
}

void load_vulkan_device_funcs(VkDevice *dev) {
	SH_FUNC_LOAD_DEV(vkGetDeviceQueue);
}


void sh_print_queue_family_flags(VkQueueFlagBits flags) {
	printf("Flags:\n");
	printf("\t%-27s = %d\n", GET_QUEUE_FLAG_NAME(VK_QUEUE_GRAPHICS_BIT), (flags & VK_QUEUE_GRAPHICS_BIT) > 0);
	printf("\t%-27s = %d\n", GET_QUEUE_FLAG_NAME(VK_QUEUE_COMPUTE_BIT), (flags & VK_QUEUE_COMPUTE_BIT) > 0);
	printf("\t%-27s = %d\n", GET_QUEUE_FLAG_NAME(VK_QUEUE_TRANSFER_BIT), (flags & VK_QUEUE_TRANSFER_BIT) > 0);
	printf("\t%-27s = %d\n", GET_QUEUE_FLAG_NAME(VK_QUEUE_SPARSE_BINDING_BIT), (flags & VK_QUEUE_SPARSE_BINDING_BIT) > 0);
	printf("\t%-27s = %d\n", GET_QUEUE_FLAG_NAME(VK_QUEUE_PROTECTED_BIT), (flags & VK_QUEUE_PROTECTED_BIT) > 0);
}
