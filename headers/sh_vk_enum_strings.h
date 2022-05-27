#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#include "glslang/glslang_c_shader_types.h"
#include "glslang/glslang_c_interface.h"
#include "sh_tools.h"

typedef struct vk_struct_type_name_t {
	char *name;
	u64 value;
} vk_struct_type_name_t;

typedef struct sh_vk_struct sh_vk_struct;
typedef struct sh_vk_struct {
	u64 type;
	sh_vk_struct *pnext;	
} sh_vk_struct;

const glslang_resource_t sh_vk_default_resource_limits = {
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .maxMeshOutputVerticesNV = */ 256,
	/* .maxMeshOutputPrimitivesNV = */ 512,
	/* .maxMeshWorkGroupSizeX_NV = */ 32,
	/* .maxMeshWorkGroupSizeY_NV = */ 1,
	/* .maxMeshWorkGroupSizeZ_NV = */ 1,
	/* .maxTaskWorkGroupSizeX_NV = */ 32,
	/* .maxTaskWorkGroupSizeY_NV = */ 1,
	/* .maxTaskWorkGroupSizeZ_NV = */ 1,
	/* .maxMeshViewCountNV = */ 4,
	/* .maxDualSourceDrawBuffersEXT = */ 1,

	/* .limits = */ {
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
	}
};

#define ADD_STRUCT_NAME(name) case name: return #name;
#define PRINT_MEMBER_FIELD(ptr, name) log_debug_morel("\t%-50s: %d", #name, ptr->name)

// Vulkan Structs Names

char* sh_get_struct_type_name(VkStructureType type);
void sh_print_vulkan_struct(void* vk_struct);
void sh_print_physical_device_limits(VkPhysicalDeviceLimits limits);

// Physical Device Features
char* sh_get_device_type_name(VkPhysicalDeviceType type);
void sh_print_pdevice_info(VkPhysicalDeviceProperties2 *p2);
void sh_print_vk_device_1_3_features(VkPhysicalDeviceVulkan13Features *vk_feat);
void sh_print_vk_pdevice_features(VkPhysicalDeviceFeatures *vk_feat);
void sh_print_pdevice_features(VkPhysicalDeviceFeatures2 *feat);

// Queue Family Flag Names
const char* sh_get_queue_flag_name(VkQueueFlagBits flags);
void sh_print_queue_family_flags(VkQueueFlagBits flags);

// Memory Property Flags Names
char* sh_get_memory_flag_name(VkMemoryPropertyFlagBits type);
void sh_print_memory_heap_flags(VkMemoryHeapFlags flags);
void sh_print_memory_type_flags(VkMemoryPropertyFlags flags);
void sh_print_memory_requirements(VkMemoryRequirements2 *mem_req);


char* sh_get_surface_composite_alpha_flag_name(VkCompositeAlphaFlagBitsKHR type);
char* sh_get_surface_transform_flag_name(VkSurfaceTransformFlagBitsKHR type);
char* sh_get_image_usage_flag_name(VkImageUsageFlagBits type);
void sh_print_surface_capabilities(VkSurfaceCapabilitiesKHR *cap);
char* sh_get_format_flag_name(VkFormat type);
char* sh_get_surface_format_color_space_name(VkColorSpaceKHR type);
char* sh_get_present_mode_name(VkPresentModeKHR type);
void sh_print_memory_type_bits(VkMemoryPropertyFlags flags);
void sh_print_memory_requirements(VkMemoryRequirements2 *mem_req);

