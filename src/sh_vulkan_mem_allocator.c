# include "sh_vulkan_mem_allocator.h"


i32 sh_get_memory_required(sh_vulkan_context_t *vk_ctx, VkMemoryPropertyFlags flags) {

	i32 selected_mem_type = -1;
	i32 mem_types_len = buf_len(vk_ctx->device_info.mem_types);

	for(i32 i = 0; i < mem_types_len; i++) {
		if((vk_ctx->device_info.mem_types[i] & flags) == flags) {
			selected_mem_type = i;
			break;
		}
	}

	if(selected_mem_type == -1)
		log_debugl("Failed to find a memory type with required flags [0x%x]", flags);

	return selected_mem_type;
}

VkMemoryRequirements2 sh_get_buf_mem_requirement(sh_vulkan_context_t *vk_ctx, VkBuffer buf) {
	VkMemoryRequirements2 mem_req = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
		.pNext = NULL
	};

	VkBufferMemoryRequirementsInfo2 buf_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2,
		.pNext = NULL,
		.buffer = buf
	};

	vkGetBufferMemoryRequirements2(vk_ctx->device_info.ldevice, &buf_info, &mem_req);

	sh_print_memory_requirements(&mem_req);

	return mem_req;
}

VkMemoryRequirements2 sh_get_img_mem_requirement(sh_vulkan_context_t *vk_ctx, VkImage img) {
	VkMemoryRequirements2 mem_req = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
		.pNext = NULL
	};

	VkImageMemoryRequirementsInfo2 buf_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2,
		.pNext = NULL,
		.image = img
	};

	vkGetImageMemoryRequirements2(vk_ctx->device_info.ldevice, &buf_info, &mem_req);

	sh_print_memory_requirements(&mem_req);

	return mem_req;
}


void sh_bind_buffer_to_memory(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *buf_alloc) {

	VkBindBufferMemoryInfo buffer_bind_info = {
		.sType = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO,
		.pNext = NULL,
		.buffer = buf_alloc->buf,
		.memory = vk_ctx->mem->device_mems[buf_alloc->device_mem_index].mem,
		.memoryOffset = buf_alloc->buffer_offset 
	};

	CHECK_VK_RESULT_MSG(
			vkBindBufferMemory2(vk_ctx->device_info.ldevice, 1, &buffer_bind_info),
			"Couldn't bind Memory"
			);
}

void sh_bind_image_to_memory(sh_vulkan_context_t *vk_ctx, sh_vk_image_allocation_t *img_alloc) {

	VkBindImageMemoryInfo buffer_bind_info = {
		.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO,
		.pNext = NULL,
		.image = img_alloc->img,
		.memory = vk_ctx->mem->device_mems[img_alloc->device_mem_index].mem,
		.memoryOffset = img_alloc->buffer_offset 
	};

	CHECK_VK_RESULT_MSG(
			vkBindImageMemory2(vk_ctx->device_info.ldevice, 1, &buffer_bind_info),
			"Couldn't bind Memory"
			);
}

VkBuffer sh_create_buffer(sh_vulkan_context_t *vk_ctx, i32 size, VkBufferUsageFlags usage) {
	VkBufferCreateInfo buf_create_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.size = size,
		.usage = usage,
		.sharingMode = 0,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &vk_ctx->queue_family_selected
	};

	VkBuffer buf_handle;

	CHECK_VK_RESULT(vkCreateBuffer(vk_ctx->device_info.ldevice, &buf_create_info, NULL, &buf_handle));

	return buf_handle;
}

VkSampler sh_create_sampler(sh_vulkan_context_t *vk_ctx, i8 enable_anisotropy) {

	VkSamplerCreateInfo sampler_create = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.magFilter = VK_FILTER_LINEAR,
		.minFilter = VK_FILTER_LINEAR,
		.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
		.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
		.mipLodBias = 0.0,
		.anisotropyEnable = enable_anisotropy == 1 ? VK_TRUE : VK_FALSE,
		.maxAnisotropy = 1.0,
		.compareEnable = VK_FALSE,
		.minLod = 0.0,
		.maxLod = VK_LOD_CLAMP_NONE,
		.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
		.unnormalizedCoordinates = VK_FALSE
	};

	VkSampler sampler;
	CHECK_VK_RESULT(vkCreateSampler(vk_ctx->device_info.ldevice, &sampler_create, NULL, &sampler));
	return sampler;
}

VkImage sh_create_2D_image(sh_vulkan_context_t *vk_ctx, i32 width, i32 height, VkImageUsageFlags usage, VkFormat format, i32 mipmap_level) {

	VkImageCreateInfo img_create = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.pNext = 0,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = {width, height, 1},
		.mipLevels = mipmap_level,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VkImage img;

	CHECK_VK_RESULT(
		vkCreateImage(
			vk_ctx->device_info.ldevice,
			&img_create,
			NULL,
			&img
		)
	);

	return img;
}

VkImageView sh_create_image_view(sh_vulkan_context_t *vk_ctx, sh_vk_image_allocation_t *image, VkImageAspectFlags aspect) {

	VkImageViewCreateInfo image_view_create = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = NULL,
		.flags = 0,
		.image = image->img,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = image->format,
		.components = {
			.r = VK_COMPONENT_SWIZZLE_R,
			.g = VK_COMPONENT_SWIZZLE_G,
			.b = VK_COMPONENT_SWIZZLE_B,
			.a = VK_COMPONENT_SWIZZLE_A
		},
		.subresourceRange = {
			.aspectMask = aspect,
			.baseMipLevel = 0,
			.levelCount = VK_REMAINING_MIP_LEVELS,
			.baseArrayLayer = 0,
			.layerCount = VK_REMAINING_ARRAY_LAYERS
		}
	};

	VkImageView image_view;
	
	CHECK_VK_RESULT(vkCreateImageView(vk_ctx->device_info.ldevice, &image_view_create, NULL, &image_view));

	return image_view;
}


VkDeviceMemory sh_create_memory(sh_vulkan_context_t *vk_ctx, VkMemoryPropertyFlags memory_requirements, VkDeviceSize size) {

	i32 memory_type_index = sh_get_memory_required(vk_ctx, memory_requirements);
	VkMemoryAllocateInfo mem_alloc = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.pNext = NULL,
		.memoryTypeIndex = memory_type_index,
		.allocationSize = size
	};



	SH_ASSERT_EXIT(memory_type_index != -1, "Memory Type Cannot be -1");

	VkDeviceMemory device_memory;
	CHECK_VK_RESULT_MSG(
		vkAllocateMemory(vk_ctx->device_info.ldevice, &mem_alloc, NULL, &device_memory),
		"Failed to allocate device memory"
	);

	return device_memory;
}

void sh_copy_buf_data_to_gpu_sync(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *mem_alloc, i32 data_offset, i32 data_size, u8 *data_to_send) {

	// TODO alloc this with alignment?
	// TODO won't work if you use device_local, you need to upload the memory and use two buffers
	u8 *mem_map = NULL;
	CHECK_VK_RESULT(vkMapMemory(vk_ctx->device_info.ldevice, SH_GET_DEVICE_MEM(mem_alloc->device_mem_index), 0, VK_WHOLE_SIZE, 0,  (void**)&mem_map));
	memcpy(mem_map, data_to_send, data_size);

	VkMappedMemoryRange mem_range = {
		.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		.pNext = NULL,
		.memory = SH_GET_DEVICE_MEM(mem_alloc->device_mem_index),
		.offset = mem_alloc->buffer_offset + data_offset,
		.size = VK_WHOLE_SIZE
	};

	vkFlushMappedMemoryRanges(vk_ctx->device_info.ldevice, 1, &mem_range);

	vkUnmapMemory(vk_ctx->device_info.ldevice, SH_GET_DEVICE_MEM( mem_alloc->device_mem_index ));
}

void sh_copy_img_data_to_gpu_sync(sh_vulkan_context_t *vk_ctx, sh_vk_image_allocation_t *mem_alloc, i32 data_offset, i32 data_size, u8 *data_to_send) {

	u8 *mem_map = NULL;
	CHECK_VK_RESULT(vkMapMemory(vk_ctx->device_info.ldevice, SH_GET_DEVICE_MEM(mem_alloc->device_mem_index), 0, VK_WHOLE_SIZE, 0,  (void**)&mem_map));
	memcpy(mem_map, data_to_send, data_size);

	VkMappedMemoryRange mem_range = {
		.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		.pNext = NULL,
		.memory = SH_GET_DEVICE_MEM(mem_alloc->device_mem_index),
		.offset = mem_alloc->buffer_offset + data_offset,
		.size = VK_WHOLE_SIZE
	};

	vkFlushMappedMemoryRanges(vk_ctx->device_info.ldevice, 1, &mem_range);

	vkUnmapMemory(vk_ctx->device_info.ldevice, SH_GET_DEVICE_MEM( mem_alloc->device_mem_index ));

}

void sh_copy_buf_to_img(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *buffer, sh_vk_image_allocation_t *image) {

	VkCommandBufferBeginInfo x = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	CHECK_VK_RESULT(vkBeginCommandBuffer(vk_ctx->temp_buffer, &x));

	sh_img_layout_transition(
		vk_ctx->temp_buffer,
		image, VK_PIPELINE_STAGE_2_NONE,
		VK_ACCESS_2_NONE,
		VK_PIPELINE_STAGE_2_COPY_BIT,
		VK_ACCESS_2_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		0,
		1
	);

	VkImageSubresourceLayers sub_res = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.mipLevel = 0,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	VkBufferImageCopy2 img_copy = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
		.pNext = 0,
		.bufferOffset = 0,
		.bufferRowLength = image->width,
		.bufferImageHeight = image->height,
		.imageSubresource = sub_res,
		.imageOffset = {0, 0, 0},
		.imageExtent = {image->width, image->height, 1}

	};

	VkCopyBufferToImageInfo2 cpy_inf = {
		.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
		.pNext = NULL,
		.srcBuffer = buffer->buf,
		.dstImage = image->img,
		.dstImageLayout =  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.regionCount = 1,
		.pRegions = &img_copy
	};

	vkCmdCopyBufferToImage2(vk_ctx->temp_buffer, &cpy_inf);

	CHECK_VK_RESULT(vkEndCommandBuffer(vk_ctx->temp_buffer););

	VkCommandBufferSubmitInfo cmd_submit_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = NULL,
		.deviceMask = 0
	};

	cmd_submit_info.commandBuffer = vk_ctx->temp_buffer;

	VkSubmitInfo2 sub_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = NULL,
		.waitSemaphoreInfoCount  = 0,
		.pWaitSemaphoreInfos = NULL,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmd_submit_info,
		.signalSemaphoreInfoCount = 0,
		.pSignalSemaphoreInfos = NULL
	};

	CHECK_VK_RESULT(vkQueueSubmit2(vk_ctx->queue, 1, &sub_info, VK_NULL_HANDLE));

	vkQueueWaitIdle(vk_ctx->queue);

	vkResetCommandPool( vk_ctx->device_info.ldevice, vk_ctx->cmd_pool, 0);
}

void sh_img_layout_transition(VkCommandBuffer cmd,
	sh_vk_image_allocation_t *image,

	VkPipelineStageFlags2 src_stage,
	VkAccessFlags2 src_access,

	VkPipelineStageFlags2 dst_stage,
	VkAccessFlags2 dst_access,

	VkImageLayout old_layout,
	VkImageLayout new_layout,
	i32 mipmap_level,
	i32 mipmap_level_count)
{

	VkImageMemoryBarrier2 img_barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.pNext = 0,
		.srcStageMask = src_stage,
		.srcAccessMask = src_access,
		.dstStageMask = dst_stage,
		.dstAccessMask = dst_access,
		.oldLayout = old_layout,
		.newLayout = new_layout,
		.image = image->img,
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = mipmap_level,
			.levelCount = mipmap_level_count,
			.baseArrayLayer = 0,
			.layerCount = VK_REMAINING_ARRAY_LAYERS
		}
	};

	VkDependencyInfo dep_info = {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.pNext = NULL,
		.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &img_barrier
	};

	SH_MARK_DEBUG_POINT(vkCmdPipelineBarrier2(cmd, &dep_info));
}

sh_vk_buffer_allocation_t sh_allocate_buffer(sh_vulkan_context_t *vk_ctx, i32 size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_type) {

	buf_fit(vk_ctx->mem->allocs, 1);
	buf_fit(vk_ctx->mem->device_mems, 1);

	sh_vk_buffer_allocation_t buf_alloc = {0};
	sh_vk_device_memory_t mem_alloc = {0};

	buf_alloc.buffer_offset = 0;
	buf_alloc.buf = sh_create_buffer(vk_ctx, size, usage);
	buf_alloc.size = size;
	VkMemoryRequirements2 mem_req = sh_get_buf_mem_requirement(vk_ctx, buf_alloc.buf);

	mem_alloc.mem = sh_create_memory(vk_ctx, memory_type, mem_req.memoryRequirements.size);
	mem_alloc.size = mem_req.memoryRequirements.size;

	buf_push(vk_ctx->mem->device_mems, mem_alloc);

	buf_alloc.device_mem_index = buf_len(vk_ctx->mem->device_mems) - 1;

	sh_bind_buffer_to_memory(vk_ctx, &buf_alloc);

	buf_push(vk_ctx->mem->allocs, buf_alloc);

	return buf_alloc;
}


void sh_deallocate_buffer(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *buf) {

	i32 mem_index = buf->device_mem_index;
	i32 buf_count = buf_len(vk_ctx->mem->allocs);

	i32 buf_index = -1;
	for(i32 i = 0; i < buf_count; i++) {
		if(vk_ctx->mem->allocs[i].buf == buf->buf){
			buf_index = i;
			break;
		}
	}

	if(buf_index > 0) {
		sh_vk_buffer_allocation_t *v = vk_ctx->mem->allocs + buf_index;
		SH_MARK_DEBUG_POINT(vkDestroyBuffer(vk_ctx->device_info.ldevice, v->buf,  NULL ));
		v->buf = VK_NULL_HANDLE;
		v->size = 0;
		v->buffer_offset = 0;
		v->device_mem_index = -1;
	}

	SH_MARK_DEBUG_POINT(
		vkFreeMemory(
			vk_ctx->device_info.ldevice,
			vk_ctx->mem->device_mems[mem_index].mem,
			NULL
		)
	);

	vk_ctx->mem->device_mems[mem_index].mem = VK_NULL_HANDLE;
	vk_ctx->mem->device_mems[mem_index].size = 0;

}




sh_vk_sampler_allocation_t sh_allocate_sampler(sh_vulkan_context_t *vk_ctx, i8 enable_anisotropy) {

	sh_vk_sampler_allocation_t alloc = {
		.handle = sh_create_sampler(vk_ctx, enable_anisotropy)
	};

	buf_push(vk_ctx->mem->sampler_allocs, alloc);

	return alloc;

}

sh_vk_image_allocation_t sh_allocate_2D_image(
	sh_vulkan_context_t *vk_ctx,
	i32 width,
	i32 height,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags memory_type,
	VkFormat format,
	i8 generate_mipmaps
	)
{

	buf_fit(vk_ctx->mem->image_allocs, 1);
	buf_fit(vk_ctx->mem->device_mems, 1);

	sh_vk_image_allocation_t img_alloc = {0};
	sh_vk_device_memory_t mem_alloc = {0};

	img_alloc.buffer_offset = 0;

	// TODO are we counting how many bits there are?

	i32 levels = 1;
	if(generate_mipmaps) {
		levels = SH_MAX(width, height);
		levels = (i32)floor(log2(levels)) + 1;
	}

	img_alloc.img = sh_create_2D_image(vk_ctx, width, height, usage, format, levels);
	img_alloc.format = format;
	img_alloc.width = width;
	img_alloc.height = height;
	img_alloc.mipmap_levels = levels;
	VkMemoryRequirements2 mem_req = sh_get_img_mem_requirement(vk_ctx, img_alloc.img);

	mem_alloc.mem = sh_create_memory(vk_ctx, memory_type, mem_req.memoryRequirements.size);
	mem_alloc.size = mem_req.memoryRequirements.size;

	buf_push(vk_ctx->mem->device_mems, mem_alloc);

	img_alloc.device_mem_index = buf_len(vk_ctx->mem->device_mems) - 1;

	sh_bind_image_to_memory(vk_ctx, &img_alloc);

	buf_push(vk_ctx->mem->image_allocs, img_alloc);

	return img_alloc;
}

sh_vk_image_view_allocation_t sh_allocate_image_view(sh_vulkan_context_t *vk_ctx, sh_vk_image_allocation_t *img, VkImageAspectFlags aspect) {

	sh_vk_image_view_allocation_t alloc = {
		.handle = sh_create_image_view(vk_ctx, img, aspect)
	};

	buf_push(vk_ctx->mem->image_view_allocs, alloc);

	return alloc;
}



sh_vk_mapped_buffer_t sh_map_vk_buffer(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *buffer, u64 size_to_map) {


	// TODO alloc this with alignment?
	// TODO won't work if you use device_local, you need to upload the memory and use two buffers

	VkDeviceSize mapped_size = buffer->size > size_to_map ? size_to_map : VK_WHOLE_SIZE;

	sh_vk_mapped_buffer_t mapped_buf = {
		.size = mapped_size == VK_WHOLE_SIZE ? buffer->size : mapped_size,
		.device_mem = SH_GET_DEVICE_MEM(buffer->device_mem_index)
	};

	u8 *mem_map = NULL;

	CHECK_VK_RESULT(
		vkMapMemory(
			vk_ctx->device_info.ldevice,
			SH_GET_DEVICE_MEM(buffer->device_mem_index),
			0, mapped_size,
			0, (void**)&mapped_buf.ptr
		)
	);

	mapped_buf.mapped = 1;

	return mapped_buf;
}


void sh_unmap_vk_buffer(sh_vulkan_context_t *vk_ctx, sh_vk_mapped_buffer_t *mapped_buf) {
	vkUnmapMemory(vk_ctx->device_info.ldevice, mapped_buf->device_mem);
	mapped_buf->mapped = 0;
}

void sh_copy_data_to_mapped_buf(sh_vk_mapped_buffer_t *buffer, u64 size, u8* data) {
	u64 copy_data_amount = buffer->size > size ? size : buffer->size;
	memcpy(buffer->ptr, data, copy_data_amount);
}

void sh_make_mapped_buf_visible(sh_vulkan_context_t *vk_ctx, sh_vk_mapped_buffer_t *buffer, u64 size_to_make_visible) {

	VkDeviceSize size_range = buffer->size > size_to_make_visible ? size_to_make_visible : buffer->size;

	if(size_to_make_visible == VK_WHOLE_SIZE)
		size_range = VK_WHOLE_SIZE;

	// TODO: add memory with offset
	VkMappedMemoryRange mem_range = {
		.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		.pNext = NULL,
		.memory = buffer->device_mem,
		.offset = 0,
		.size = size_range
	};

	vkFlushMappedMemoryRanges(vk_ctx->device_info.ldevice, 1, &mem_range);
	vkInvalidateMappedMemoryRanges(vk_ctx->device_info.ldevice, 1, &mem_range);

}
void sh_copy_buf_to_buf_sync(sh_vulkan_context_t *vk_ctx, sh_vk_buffer_allocation_t *src, sh_vk_buffer_allocation_t *dst, u64 data_size, u64 dst_offset) {

	VkCommandBufferBeginInfo x = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	CHECK_VK_RESULT(vkBeginCommandBuffer(vk_ctx->temp_buffer, &x));

	
	//TODO: handle offsets better
	VkBufferCopy2 buf_copy = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
		.pNext = NULL, 
		.srcOffset = src->buffer_offset,
		.dstOffset = dst_offset,
		.size = data_size
	};

	VkCopyBufferInfo2 buf_buf_copy = {
		.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
		.pNext = NULL,
		.srcBuffer = src->buf,
		.dstBuffer = dst->buf,
		.regionCount = 1,
		.pRegions = &buf_copy
	};

	SH_MARK_DEBUG_POINT(vkCmdCopyBuffer2(vk_ctx->temp_buffer, &buf_buf_copy));

	CHECK_VK_RESULT(vkEndCommandBuffer(vk_ctx->temp_buffer););

	VkCommandBufferSubmitInfo cmd_submit_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = NULL,
		.deviceMask = 0
	};

	cmd_submit_info.commandBuffer = vk_ctx->temp_buffer;

	VkSubmitInfo2 sub_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = NULL,
		.waitSemaphoreInfoCount  = 0,
		.pWaitSemaphoreInfos = NULL,
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmd_submit_info,
		.signalSemaphoreInfoCount = 0,
		.pSignalSemaphoreInfos = NULL
	};

	CHECK_VK_RESULT(vkQueueSubmit2(vk_ctx->queue, 1, &sub_info, VK_NULL_HANDLE));
	vkQueueWaitIdle(vk_ctx->queue);
	vkResetCommandPool( vk_ctx->device_info.ldevice, vk_ctx->cmd_pool, 0);
}

