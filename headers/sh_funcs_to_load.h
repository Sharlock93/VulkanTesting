#define SH_VK_GLOBAL_FUNCS\
	FUNC(vkGetInstanceProcAddr)\
	FUNC(vkGetDeviceProcAddr)\
	FUNC(vkCreateInstance)\
	FUNC(vkDestroyInstance)\
	FUNC(vkEnumerateInstanceVersion)\
	FUNC(vkEnumerateInstanceExtensionProperties)\
	FUNC(vkEnumerateInstanceLayerProperties)\
	FUNC(vkEnumeratePhysicalDevices)\
	
#define SH_VK_INSTANCE_FUNCS\
	FUNC(vkEnumeratePhysicalDevices)\
	FUNC(vkGetPhysicalDeviceProperties)\
	FUNC(vkGetPhysicalDeviceProperties2)\
	FUNC(vkGetPhysicalDeviceFeatures)\
	FUNC(vkGetPhysicalDeviceFeatures2)\
	FUNC(vkDestroyInstance)\
	FUNC(vkCreateDevice)\
	FUNC(vkDestroyDevice)\
	FUNC(vkEnumerateDeviceExtensionProperties)\
	FUNC(vkCreateDebugUtilsMessengerEXT)\
	FUNC(vkDestroyDebugUtilsMessengerEXT)\
	FUNC(vkGetPhysicalDeviceQueueFamilyProperties)\
	FUNC(vkGetPhysicalDeviceQueueFamilyProperties2)\
	FUNC(vkGetPhysicalDeviceMemoryProperties2)\
	FUNC(vkCreateWin32SurfaceKHR)\
	FUNC(vkGetPhysicalDeviceWin32PresentationSupportKHR)\
	FUNC(vkDestroySurfaceKHR)\
	FUNC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)\
	FUNC(vkGetPhysicalDeviceSurfaceCapabilities2KHR)\
	FUNC(vkGetPhysicalDeviceSurfaceFormatsKHR)\
	FUNC(vkGetPhysicalDeviceSurfaceFormats2KHR)\
	FUNC(vkGetPhysicalDeviceSurfacePresentModesKHR)\
	FUNC(vkGetPhysicalDeviceSurfacePresentModes2EXT)\
	FUNC(vkGetPhysicalDeviceSurfaceSupportKHR)\
	FUNC(vkGetPhysicalDevicePresentRectanglesKHR)\
	FUNC(vkGetPhysicalDeviceImageFormatProperties2)\
	FUNC(vkEnumeratePhysicalDeviceGroups)\
	FUNC(vkCreateFramebuffer)\
	FUNC(vkDestroyFramebuffer)\

#define SH_VK_DEVICE_FUNCS\
	FUNC(vkGetDeviceQueue)\
	FUNC(vkGetDeviceQueue2)\
	FUNC(vkDestroyDevice)\
	FUNC(vkAcquireNextImageKHR)\
	FUNC(vkCreateSwapchainKHR)\
	FUNC(vkDestroySwapchainKHR)\
	FUNC(vkGetSwapchainImagesKHR)\
	FUNC(vkQueuePresentKHR)\
	FUNC(vkAcquireNextImage2KHR)\
	FUNC(vkGetDeviceGroupPresentCapabilitiesKHR)\
	FUNC(vkGetDeviceGroupSurfacePresentModesKHR)\
	FUNC(vkCreateImageView)\
	FUNC(vkDestroyImageView)\
	FUNC(vkCreateShaderModule)\
	FUNC(vkDestroyShaderModule)\
	FUNC(vkCreatePipelineLayout)\
	FUNC(vkDestroyPipelineLayout)\
	FUNC(vkCreateGraphicsPipelines)\
	FUNC(vkDestroyPipeline)\
	FUNC(vkCreateCommandPool)\
	FUNC(vkResetCommandPool)\
	FUNC(vkDestroyCommandPool)\
	FUNC(vkAllocateCommandBuffers)\
	FUNC(vkBeginCommandBuffer)\
	FUNC(vkCreateSemaphore)\
	FUNC(vkDestroySemaphore)\
	FUNC(vkAcquireNextImageKHR)\
	FUNC(vkQueueSubmit)\
	FUNC(vkQueueSubmit2)\
	FUNC(vkCreateRenderPass)\
	FUNC(vkCreateRenderPass2)\
	FUNC(vkDestroyRenderPass)\
	FUNC(vkCmdBeginRenderPass2)\
	FUNC(vkCmdEndRenderPass2)\
	FUNC(vkCmdBeginRenderPass)\
	FUNC(vkCmdBindPipeline)\
	FUNC(vkCmdDraw)\
	FUNC(vkCmdDrawIndexed)\
	FUNC(vkCmdEndRenderPass)\
	FUNC(vkCmdBlitImage2)\
	FUNC(vkCmdBindVertexBuffers)\
	FUNC(vkCmdBindIndexBuffer)\
	FUNC(vkCmdBindVertexBuffers2)\
	FUNC(vkEndCommandBuffer)\
	FUNC(vkQueuePresentKHR)\
	FUNC(vkQueueWaitIdle)\
	FUNC(vkCreateBuffer)\
	FUNC(vkDestroyBuffer)\
	FUNC(vkGetBufferMemoryRequirements2)\
	FUNC(vkGetImageMemoryRequirements2)\
	FUNC(vkCmdUpdateBuffer)\
	FUNC(vkAllocateMemory)\
	FUNC(vkFreeMemory)\
	FUNC(vkBindBufferMemory2)\
	FUNC(vkBindImageMemory2)\
	FUNC(vkMapMemory)\
	FUNC(vkUnmapMemory)\
	FUNC(vkFlushMappedMemoryRanges)\
	FUNC(vkInvalidateMappedMemoryRanges)\
	FUNC(vkCmdPipelineBarrier2)\
	FUNC(vkCreateDescriptorSetLayout)\
	FUNC(vkDestroyDescriptorSetLayout)\
	FUNC(vkCreateDescriptorPool)\
	FUNC(vkDestroyDescriptorPool)\
	FUNC(vkAllocateDescriptorSets)\
	FUNC(vkFreeDescriptorSets)\
	FUNC(vkUpdateDescriptorSets)\
	FUNC(vkCmdBindDescriptorSets)\
	FUNC(vkCreateImage)\
	FUNC(vkCmdCopyBufferToImage2)\
	FUNC(vkCmdCopyBuffer2)\
	FUNC(vkDestroyImage)\
	FUNC(vkCreateSampler)\
	FUNC(vkDestroySampler)\


#define FUNC(name) PFN_##name name;

SH_VK_GLOBAL_FUNCS;
SH_VK_INSTANCE_FUNCS;
SH_VK_DEVICE_FUNCS;

#undef FUNC

#define FUNC(name) { (void**)&name, #name },

typedef struct sh_vk_funcname {
	void **func_ptr;
	char *name;
} sh_vk_funcname;

sh_vk_funcname sh_vk_global_funcs[] = {
	SH_VK_GLOBAL_FUNCS
};

sh_vk_funcname sh_vk_instance_funcs[] = {
	SH_VK_INSTANCE_FUNCS
};

sh_vk_funcname sh_vk_device_funcs[] = {
	SH_VK_DEVICE_FUNCS
};

#undef FUNC
