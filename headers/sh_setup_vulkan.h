VKAPI_ATTR VkBool32 VKAPI_CALL debug_dump_everything(
	VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_type,
	const VkDebugUtilsMessengerCallbackDataEXT *data,
	void *user_data);

VkBool32 VKAPI_PTR create_instance_debug_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT *data,
		void *user_data);


void sh_setup_debug_callbacks(sh_vulkan_context_t *vk_ctx);
sh_vulkan_context_t* sh_setup_vulkan_instance(sh_vulkan_context_t *vk_ctx, sh_pdev_select_func phy_device_selector);
void sh_setup_physical_device_groups(sh_vulkan_context_t *vk_ctx);
void sh_setup_surface_and_rendering(sh_window_context_t *wn, sh_vulkan_context_t *vk_ctx);
void sh_setup_queue_family(sh_vulkan_context_t *vk_ctx, sh_qfamily_select_func qsel_func);
void sh_setup_swapchain(sh_window_context_t *win_ctx, sh_vulkan_context_t *vk_ctx);

void sh_destroy_command_pool(sh_vulkan_context_t *vk_ctx);
void sh_destroy_framebuffers(sh_vulkan_context_t *vk_ctx);
void sh_destroy_image_view(sh_vulkan_context_t *vk_ctx);
void sh_destroy_graphics_pipeline(sh_vulkan_context_t* vk_ctx);
void sh_destroy_device_memory(sh_vulkan_context_t *vk_ctx);
void sh_destroy_descriptors(sh_vulkan_context_t *vk_ctx);
void destroy_vulkan_instance(sh_vulkan_context_t *vk_ctx);
void sh_create_render_pass(
	sh_vulkan_context_t *vk_ctx,
	i32 pipeline_attachment_count,
	VkAttachmentDescription2 *attachments,
	i32 color_attachment_count,
	VkAttachmentReference2 *color_attach_refs,
	VkAttachmentReference2 *depth_attach_refs,
	i32 input_attachment_count,
	VkAttachmentReference2 *input_attach_refs,
	VkRenderPass *render_pass,
	i32 subpass_dependency_count,
	VkSubpassDependency2 *subpass_dependency
	);
void sh_setup_render_pass(sh_vulkan_context_t *vk_ctx);

void sh_setup_shader_modules(sh_vulkan_context_t *vk_ctx, sh_vk_spirv_shader_t *shaders, u32 shaders_count);
void sh_setup_descriptor_sets(sh_vulkan_context_t* vk_ctx);

void sh_create_graphics_pipeline(
	sh_window_context_t *wn_ctx,
	sh_vulkan_context_t *vk_ctx,
	i32 shaders_count,
	sh_vk_shader_module_t *shaders,
	i32 v_in_bindings_count,
	VkVertexInputBindingDescription *v_in_bindings,
	i32 v_in_attr_count,
	VkVertexInputAttributeDescription *v_in_attr,
	i32 blend_attachments_count,
	VkPipelineColorBlendAttachmentState *blend_attachments,
	i32 set_layouts_count,
	VkDescriptorSetLayout *set_layouts,
	VkPipelineLayout *layout,
	VkRenderPass render_pass,
	VkPipeline *graphics_pipeline
);

void sh_create_framebuffers(
	sh_window_context_t *win_ctx,
	sh_vulkan_context_t *vk_ctx,
	i32 view_count,
	VkImageView *views,
	VkRenderPass render_pass,
	u32 frame_buffer_count,
	VkFramebuffer *framebuffers
	);

void sh_setup_graphics_pipeline(sh_window_context_t *wn_ctx, sh_vulkan_context_t *vk_ctx);

void sh_setup_framebuffers(sh_window_context_t *win_ctx, sh_vulkan_context_t *vk_ctx);
void sh_setup_cmd_pools_and_buffers(sh_vulkan_context_t *vk_ctx);

void sh_begin_render(VkCommandBuffer buffer);
void sh_begin_render_pass(sh_window_context_t *win_ctx, sh_vulkan_context_t *vk_ctx, VkCommandBuffer cmd_buffer, VkFramebuffer frame_buffer, VkRenderPass render_pass, i32 clear_val_count, VkClearValue *clear_vals);
void sh_end_render_pass(VkCommandBuffer cmd_buffer);
void sh_setup_cmd_render(sh_window_context_t *win_ctx, sh_vulkan_context_t *vk_ctx);
void sh_setup_semaphores(sh_vulkan_context_t *vk_ctx);
void sh_recreate_rendering_resources(sh_window_context_t* ctx, sh_vulkan_context_t *vk_ctx);
void sh_setup_device_memory(sh_vulkan_context_t *vk_ctx);
sh_window_context_t* sh_win_setup(void);

sh_vulkan_context_t* sh_vk_setup(sh_window_context_t *win_ctx, sh_pdev_select_func physical_device_selector, sh_qfamily_select_func queue_family_selector, sh_vk_spirv_shader_t* spirv_shaders);
