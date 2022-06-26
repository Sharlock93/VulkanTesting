#pragma once

#include <vulkan/vulkan.h>

typedef struct sh_vk_device_memory_t {
	VkDeviceMemory mem;
	VkDeviceSize size;
} sh_vk_device_memory_t;


typedef struct sh_vk_buffer_allocation_t {
	i32 device_mem_index;
	VkBuffer buf;
	i32 buffer_offset;
	VkDeviceSize size;
} sh_vk_buffer_allocation_t;

typedef struct sh_vk_image_allocation_t {
	i32 device_mem_index;
	VkImage img;
	i32 buffer_offset;
	VkFormat format;
	i32 width;
	i32 height;
	u32 mipmap_levels;
	i8 mipmap_generated;
} sh_vk_image_allocation_t;

typedef struct sh_vk_sampler_allocation_t {
	VkSampler handle;
} sh_vk_sampler_allocation_t;

typedef struct sh_vk_image_view_allocation_t {
	VkImageView handle;
} sh_vk_image_view_allocation_t;

typedef struct sh_vk_mapped_buffer_t {
	u8 *ptr;
	i8 mapped;
	VkDeviceSize size;
	VkDeviceMemory device_mem;
} sh_vk_mapped_buffer_t;

#define SH_GET_DEVICE_MEM(index) vk_ctx->mem->device_mems[index].mem

VkBuffer               sh_create_buffer(sh_vulkan_context_t *vk_ctx, i32 size, VkBufferUsageFlags usage);
VkSampler			   sh_create_sampler(sh_vulkan_context_t *vk_ctx, i8 enable_anisotropy);
VkImage                sh_create_2D_image(sh_vulkan_context_t *vk_ctx, i32 width, i32 height, VkImageUsageFlags usage, VkFormat format, i32 mipmap_level);
VkImageView            sh_create_image_view(sh_vulkan_context_t *vk_ctx, sh_vk_image_allocation_t *image, VkImageAspectFlags aspect);
i32                    sh_get_memory_required(sh_vulkan_context_t *vk_ctx, VkMemoryPropertyFlags flags);
VkMemoryRequirements2 sh_get_buf_mem_requirement(sh_vulkan_context_t *vk_ctx, VkBuffer buf);
VkMemoryRequirements2 sh_get_img_mem_requirement(sh_vulkan_context_t *vk_ctx, VkImage buf);
VkDeviceMemory         sh_create_memory(sh_vulkan_context_t *vk_ctx, VkMemoryPropertyFlags memory_requirements, VkDeviceSize size);
void                   sh_bind_buffer_to_memory(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *buf_alloc);
void                   sh_bind_image_to_memory(sh_vulkan_context_t *vk_ctx, sh_vk_image_allocation_t *img_alloc);
void                   sh_copy_buf_data_to_gpu_sync(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *mem_alloc, i32 data_offset, i32 data_size, u8 *data_to_send);
void                   sh_copy_img_data_to_gpu_sync(sh_vulkan_context_t *vk_ctx, sh_vk_image_allocation_t *mem_alloc, i32 data_offset, i32 data_size, u8 *data_to_send);
void                   sh_copy_buf_to_img(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *buffer, sh_vk_image_allocation_t *image);

void 				   sh_img_layout_transition(VkCommandBuffer cmd, sh_vk_image_allocation_t *image, VkPipelineStageFlags2 src_stage, VkAccessFlags2 src_access, VkPipelineStageFlags2 dst_stage, VkAccessFlags2 dst_access, VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect, i32 mipmap_level, i32 mipmap_level_count);

sh_vk_buffer_allocation_t sh_allocate_buffer(sh_vulkan_context_t *vk_ctx, i32 size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memtype);
void sh_deallocate_buffer(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *buf);
sh_vk_sampler_allocation_t sh_allocate_sampler(sh_vulkan_context_t *vk_ctx, i8 enalbe_anisotropy);
sh_vk_image_allocation_t sh_allocate_2D_image(sh_vulkan_context_t *vk_ctx, i32 width, i32 height, VkImageUsageFlags usage, VkMemoryPropertyFlags mem_type, VkFormat format, i8 generate_mipmap);
sh_vk_image_view_allocation_t sh_allocate_image_view(sh_vulkan_context_t *vk_ctx, sh_vk_image_allocation_t *img, VkImageAspectFlags aspect);


void sh_deallocate_2D_image(sh_vulkan_context_t *vk_ctx, sh_vk_image_allocation_t *img);
void sh_deallocate_image_view(sh_vulkan_context_t *vk_ctx, sh_vk_image_view_allocation_t* img_view);

sh_vk_mapped_buffer_t sh_map_vk_buffer(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *buffer, u64 size_to_map);
void sh_unmap_vk_buffer(sh_vulkan_context_t *vk_ctx, sh_vk_mapped_buffer_t *mapped_buf);
void sh_copy_data_to_mapped_buf(sh_vk_mapped_buffer_t *buffer, u64 size, u8* data);
void sh_make_mapped_buf_visible(sh_vulkan_context_t *vk_ctx, sh_vk_mapped_buffer_t *buffer, u64 size_to_make_visible);
void sh_copy_buf_to_buf_sync(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *src, sh_vk_buffer_allocation_t *dst, u64 data_size, u64 dst_offset);


typedef struct sh_vk_memory_manager_t {
	SH_BUFFER_TYPE(sh_vk_device_memory_t*) device_mems;
	SH_BUFFER_TYPE(sh_vk_buffer_allocation_t*) allocs;
	SH_BUFFER_TYPE(sh_vk_image_allocation_t*) image_allocs;
	SH_BUFFER_TYPE(sh_vk_image_view_allocation_t*) image_view_allocs;
	SH_BUFFER_TYPE(sh_vk_sampler_allocation_t*) sampler_allocs;
	sh_vk_buffer_allocation_t *staging_buffer;
} sh_vk_memory_manager_t;
