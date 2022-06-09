#include "sh_vk_tools.h"

void sh_buf_mem_sync_copy(VkCommandBuffer cmd_buf, VkPipelineStageFlags2 stage, VkAccessFlags2 access) {

	VkMemoryBarrier2 mem_barrier = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
		.pNext = NULL,
		.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstStageMask = stage,
		.dstAccessMask = access
	};

	VkDependencyInfo dep_info = {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.pNext = NULL,
		.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
		.memoryBarrierCount = 1,
		.pMemoryBarriers = &mem_barrier,
		.bufferMemoryBarrierCount = 0,
		.imageMemoryBarrierCount = 0
	};

	vkCmdPipelineBarrier2(cmd_buf, &dep_info);

}

void sh_queue_submit(
		VkQueue queue,
		u32 wait_semphs_count,
		VkSemaphore *wait_semphs,
		VkPipelineStageFlags *wait_stages,
		u32 cmd_buf_count,
		VkCommandBuffer *cmd_bufs,
		u32 signal_semphs_count,
		VkSemaphore *signal_semphs,
		VkPipelineStageFlags *signal_stages)
{

	// we will reuse across frames
	static VkSemaphoreSubmitInfo *semph_infos = NULL;
	buf_clear(semph_infos);
	buf_fit(semph_infos, wait_semphs_count + signal_semphs_count);
	VkSemaphoreSubmitInfo semph_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.pNext = NULL,
		.deviceIndex = 0
	};

	for(u32 i = 0; i < wait_semphs_count; i++) {
		semph_info.semaphore = wait_semphs[i];
		semph_info.stageMask = wait_stages[i];
		buf_push(semph_infos, semph_info);
	}

	// abuse the fact that both wait and signal semphs are the same struct so put them in the same array

	for(u32 i = 0; i < signal_semphs_count; i++) {
		semph_info.semaphore = signal_semphs[i];
		semph_info.stageMask = signal_stages[i];
		buf_push(semph_infos, semph_info);
	}

	VkCommandBufferSubmitInfo *cmd_submit_infos = NULL;
	buf_fit(cmd_submit_infos, cmd_buf_count);

	VkCommandBufferSubmitInfo cmd_submit_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = NULL,
		.deviceMask = 0
	};

	for(u32 i = 0; i < cmd_buf_count; i++) {
		cmd_submit_info.commandBuffer = cmd_bufs[i];
		buf_push(cmd_submit_infos, cmd_submit_info);
	}

	VkSubmitInfo2 sub_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
		.pNext = NULL,

		.waitSemaphoreInfoCount  = wait_semphs_count,
		.pWaitSemaphoreInfos = semph_infos,

		.commandBufferInfoCount = cmd_buf_count,
		.pCommandBufferInfos = cmd_submit_infos,

		.signalSemaphoreInfoCount = signal_semphs_count,
		.pSignalSemaphoreInfos = semph_infos + wait_semphs_count
	};

	CHECK_VK_RESULT(vkQueueSubmit2(queue, 1, &sub_info, VK_NULL_HANDLE));
}

void sh_generate_img_mipmap(sh_vulkan_context_t *vk_ctx, sh_vk_image_allocation_t *img) {

	VkImageBlit2 *regions = NULL;

	VkImageBlit2 template = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		.pNext = NULL,
		.srcSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseArrayLayer = 0,
			.layerCount = 1,
			.mipLevel = 0
		},
		.srcOffsets = {
			{0, 0, 0},
			{img->width, img->height, 1 }
		},
		.dstSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.dstOffsets = {img->width}
	};

	template.dstOffsets[0].x = 0;
	template.dstOffsets[0].y = 0;
	template.dstOffsets[0].z = 0;

	buf_fit(regions, img->mipmap_levels);
	for(u32 i = 1; i < img->mipmap_levels; i++) {
		template.dstSubresource.mipLevel = i;
		template.dstOffsets[1].x = (i32)( SH_MAX(img->width/(pow(2, i)), 1) );
		template.dstOffsets[1].y = (i32)( SH_MAX(img->height/(pow(2, i)), 1) );

		template.dstOffsets[1].z = 1;
		buf_push(regions, template);
	}

	VkBlitImageInfo2 blit_info = {
		.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		.pNext = NULL,
		.srcImage = img->img,
		.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.dstImage = img->img,
		.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.regionCount = buf_len(regions),
		.pRegions = regions
	};

	VkCommandBufferBeginInfo x = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = NULL,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};
	CHECK_VK_RESULT(vkBeginCommandBuffer(vk_ctx->temp_buffer, &x));

	sh_img_layout_transition(
		vk_ctx->temp_buffer,
		img,
		VK_PIPELINE_STAGE_2_NONE,
		VK_ACCESS_2_NONE,
		VK_PIPELINE_STAGE_2_BLIT_BIT,
		VK_ACCESS_2_TRANSFER_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT,
		0, 1
	);

	sh_img_layout_transition(
		vk_ctx->temp_buffer,
		img,
		VK_PIPELINE_STAGE_2_NONE,
		VK_ACCESS_2_NONE,
		VK_PIPELINE_STAGE_2_BLIT_BIT,
		VK_ACCESS_2_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT,

		1, VK_REMAINING_MIP_LEVELS
	);
	
	vkCmdBlitImage2(vk_ctx->temp_buffer, &blit_info);


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
	vkResetCommandPool(vk_ctx->device_info.ldevice, vk_ctx->cmd_pool, 0);
	buf_free(regions);
}
