void sh_buf_mem_sync_copy(VkCommandBuffer cmd_buf, VkPipelineStageFlags2 stage, VkAccessFlags2 access);
void sh_queue_submit(
		VkQueue              queue,
		u32                  wait_semphs_count,
		VkSemaphore          *wait_semphs,
		VkPipelineStageFlags *wait_stages,
		u32                  cmd_buf_count,
		VkCommandBuffer      *cmd_bufs,
		u32                  signal_semphs_count,
		VkSemaphore          *signal_semphs,
		VkPipelineStageFlags *signal_stages);

void sh_generate_img_mipmap(sh_vulkan_context_t *vk_ctx, sh_vk_image_allocation_t *img);
