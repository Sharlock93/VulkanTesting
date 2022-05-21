#ifndef SH_LOAD_VULKAN_H
#define SH_LOAD_VULKAN_H

#define VK_NO_PROTOTYPES

#include <vulkan/vulkan.h>
#include "vulkan_enum_strings.c"
#include "vulkan_extensions_to_enable.h"
#include <glslang/glslang_c_interface.h>
#include <glslang/glslang_c_shader_types.h>
// #include "sh_vulkan_mem_allocator.h"

#define LOAD_VK 1

#define CHECK_VK_RESULT(func_call) {\
	sh_global_log_tracker.line_number = __LINE__;\
	sh_global_log_tracker.function_name = __func__ ;\
	sh_global_log_tracker.file_name = __FILE__;\
	VkResult r = func_call;\
	if(r != VK_SUCCESS) {\
		log_debugl("VK call failed: %s", #func_call );\
        exit(1);\
	}\
	sh_global_log_tracker.line_number = 0;\
	sh_global_log_tracker.function_name = "" ;\
	sh_global_log_tracker.file_name = "";\
}

#define CHECK_VK_RESULT_MSG(func_call, msg) {\
	sh_global_log_tracker.line_number = __LINE__;\
	sh_global_log_tracker.function_name = __func__ ;\
	sh_global_log_tracker.file_name = __FILE__;\
	VkResult r = func_call;\
	if(r != VK_SUCCESS) {\
		log_debugl("VK call failed: %s - %s", __FILE__, __LINE__, #func_call, msg );\
		exit(1);\
	}\
	sh_global_log_tracker.line_number = 0;\
	sh_global_log_tracker.function_name = "" ;\
	sh_global_log_tracker.file_name = "";\
}

#if LOAD_VK
	#define SH_VK_FUNC(name) PFN_##name name
	#define SH_LOAD_VK_GET_INST_PROC_ADDR(name) name = (PFN_##name)GetProcAddress(vulkan_lib, #name);\
		if(name == NULL) {\
			log_debugl("Function Loading Failed: %s", #name, __FILE__, __LINE__);\
			exit(1);\
		}
#else
	#define SH_LOAD_VK_GET_INST_PROC_ADDR(name)
    #define SH_VK_FUNC(name)
#endif


#include "sh_funcs_to_load.h"

#undef SH_VK_FUNC

typedef struct sh_vk_memory_manager_t sh_vk_memory_manager_t;

typedef struct sh_vulkan_pdevice {
	VkPhysicalDevice device;
	VkPhysicalDeviceFeatures2 *features; //reenable if needed
	VkPhysicalDeviceProperties2 *properties; //reenable if needed TODO(sharo): if reenabled make sure to update sh_select_physical_device
	VkDevice ldevice;
	// VkQueueFamilyProperties2 *queue_properties; //reanble and check queue family selector
	SH_BUFFER_TYPE( VkMemoryPropertyFlags *) mem_types;
} sh_vulkan_pdevice;

typedef struct sh_vk_shader_module_t {
	VkShaderModule shader_module;
	VkShaderStageFlagBits stage;
} sh_vk_shader_module_t;

typedef struct sh_vulkan_context_t {

	sh_vulkan_pdevice device_info;
	VkInstance instance;
    VkQueue queue;
	VkDebugUtilsMessengerEXT debug_msgr;
	u32 queue_family_selected;

	const char** layers_enabled;
	const char** extensions_enabled;
	const char** pdevice_extensions_enabled;

	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkImageView *img_views;
    VkFormat *img_formats;
    VkPipelineLayout layout;
    VkRenderPass render_pass;
    VkPipeline pipeline;

    VkFramebuffer *framebuffers;
    VkCommandPool cmd_pool;
    VkCommandBuffer *cmd_buffers;
    VkCommandBuffer temp_buffer;

    VkSemaphore render_semaphore;
    VkSemaphore present_semaphore;
	sh_vk_shader_module_t *shader_modules;

	VkDescriptorSetLayout *set_layouts;
	VkDescriptorSet 	  *descriptor_sets;
	VkDescriptorPool 	  *descriptor_pools;

	sh_vk_memory_manager_t *mem;

	f32 clear_color[4];

} sh_vulkan_context_t;

typedef struct sh_vk_spirv_shader_t {
	u32 *data;	
	u64 size;
	VkShaderStageFlagBits stage;
} sh_vk_spirv_shader_t;

typedef enum sh_shader_type_t {
	SH_SHADER_TYPE_INVALID,
	SH_SHADER_TYPE_VERTEX,
	SH_SHADER_TYPE_FRAGMENT,
	SH_SHADER_TYPE_LAST_TYPE,
} sh_shader_type_t;

typedef struct sh_shader_input_t {
	sh_shader_type_t type;
	const char *filename;
} sh_shader_input_t;

VkShaderStageFlagBits _sh_type_to_stage_map[] = {
	[SH_SHADER_TYPE_INVALID] = -1,
	[SH_SHADER_TYPE_VERTEX] = VK_SHADER_STAGE_VERTEX_BIT,
	[SH_SHADER_TYPE_FRAGMENT] = VK_SHADER_STAGE_FRAGMENT_BIT,
};


typedef i8 (*sh_pdev_select_func)(VkPhysicalDeviceProperties device);
typedef i8 (*sh_qfamily_select_func)(sh_vulkan_context_t *vk_ctx, VkQueueFamilyProperties2 *properties, u32 queue_index);

void sh_load_vulkan_funcs(void);
void sh_load_vulkan_instance_funcs(VkInstance *inst);
void sh_load_vulkan_device_funcs(VkDevice *dev);
const char** sh_get_enabled_layers(const char **requested_layers, u32 requested_layers_count, u32 *enabled_layers_count);
const char** sh_get_enabled_instance_ext(const char** requested_extensions, u32 requested_ext_count, u32 *enabled_ext);
const char** sh_get_enabled_device_ext(sh_vulkan_context_t *vk_ctx, const char** requested_extensions, u32 requested_ext_count, u32 *enabled_ext);
void sh_select_physical_device(sh_vulkan_context_t *vk_ctx, sh_pdev_select_func select_function);
void sh_select_device_queue_family(sh_vulkan_context_t *vk_ctx, sh_qfamily_select_func select_function);

inline VkShaderStageFlagBits sh_shader_type_to_vk_stage(sh_shader_type_t type);
sh_vk_spirv_shader_t sh_compile_shader_input(sh_shader_input_t *shader);


#endif // SH_LOAD_VULKAN_H
