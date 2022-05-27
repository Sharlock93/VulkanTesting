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
