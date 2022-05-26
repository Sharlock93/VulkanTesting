/* TODO:
 *	a nice way to turn this into a sort of a framework?
 *		idea 1: push the information you want into arrays?
 *			- i.e: if you want 10 descritor sets you do sh_push_descriptor_set(vk_ctx)
 *			- then in the end you do sh_vk_init(vk_ctx) and build the entire system?
 *			- gives you the possibility to define things early on and in any order 
 *			- but build everything together at the end
 */

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR

#define ENABLE_LAYERS 1
#define ENABLE_EXTENSIONS 1
#define ENABLE_PDEVICE_EXTENSIONS 1

#include <Windows.h>
#include <dbgHelp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>

#include "sh_tools.c"
#include "sh_simple_vec_math.c"
#include "sh_types.h"
#include "sh_win_platform.c"
#include "sh_load_vulkan.c"
#include "sh_vulkan_mem_allocator.c"
#include "sh_setup_vulkan.c"
#include "sh_objloader.c"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobject/tinyobj_loader_c.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

i8 physical_device_selector(VkPhysicalDeviceProperties properties) {
	if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		return 1;
	}
	return 0;
}

i8 queue_family_selector(sh_vulkan_context_t *vk_ctx, VkQueueFamilyProperties2 *properties, u32 queue_index) {

	VkBool32 present_supported = 0;
	CHECK_VK_RESULT(
			vkGetPhysicalDeviceSurfaceSupportKHR(vk_ctx->device_info.device,
				queue_index,
				vk_ctx->surface,
				&present_supported
			)
	);

	log_debugl("Presentation Supported: %s", present_supported ? "Yes" : "No" );

	if(present_supported && (properties->queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
		return 1;
	}

	return 0;
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

typedef struct sh_camera {
	sh_vec3 eye;
	sh_vec3 fwd;
	sh_vec3 up;
	sh_mat4 mat;
} sh_camera;


sh_camera sh_setup_camera(sh_vec3 *eye, sh_vec3 *look_at, sh_vec3 *up) {

	sh_vec3 fwd = sh_vec3_new_sub_vec3(eye, look_at);
	sh_vec3_normalize_ref(&fwd);
	sh_mat4 cam =  sh_lookat(eye, look_at, up);
	sh_mat4_transpose(&cam);

	return (sh_camera){.eye = *eye, .fwd = fwd, .up = *up, .mat = cam};
}

void sh_camera_move_fwd(sh_camera *cam, f32 amount) {
	sh_vec3 translate = sh_vec3_new_mul_scaler(&cam->fwd, amount);
	sh_vec3_add_vec3(&cam->eye, &translate);
	sh_mat4_translate_vec3(&cam->mat, &translate);
}

void sh_camera_move_hor(sh_camera *cam, f32 amount) {
	sh_vec3 right = sh_vec3_cross(&cam->fwd, &cam->up);
	sh_vec3 translate = sh_vec3_new_mul_scaler(&right, amount);
	sh_vec3_add_vec3(&cam->eye, &translate);
	sh_mat4_translate_vec3(&cam->mat, &translate);
}

void sh_camera_move_vert(sh_camera *cam, f32 amount) {
	sh_vec3 translate = sh_vec3_new_mul_scaler(&cam->up, amount);
	sh_vec3_add_vec3(&cam->eye, &translate);
	sh_mat4_translate_vec3(&cam->mat, &translate);
}

void sh_camera_lookat_mouse(sh_camera *cam, f32 x_delta, f32 y_delta) {
	sh_vec3 right = sh_vec3_cross(&cam->fwd, &cam->up);
	sh_vec3_normalize_ref(&right);

	sh_mat4 rot = sh_make_mat4_axis_rot(&right, y_delta);
	sh_mat4 rot_up = sh_make_mat4_y_rot(x_delta);

	sh_mul_mat4_vec3(&rot_up, &cam->fwd);
	sh_mul_mat4_vec3(&rot, &cam->fwd);

	sh_mul_mat4_mat4(&cam->mat, &rot_up);
	sh_mul_mat4_mat4(&cam->mat, &rot);
}

void sh_read_obj_file(void *ctx, const char *file, int is_mtl, const char *obj_filename, char **buf, size_t *len) {
	u64 siz = 0;
	(*buf) = sh_read_file(file, &siz);
	*len = siz;
}

SH_BUFFER_TYPE(sh_vk_buffer_allocation_t*)
sh_make_buffer_from_obj_vertices(sh_vulkan_context_t *vk_ctx, tinyobj_attrib_t *mesh, tinyobj_material_t *materials, u64 material_count, i8 color_from_material) {

	sh_vk_buffer_allocation_t *mesh_buffers = NULL;

	sh_vertex *vs = NULL;
	i32 *indices = 0;

	buf_fit(vs, mesh->num_face_num_verts);
	buf_fit(indices, mesh->num_faces);

	for(u32 i = 0; i < mesh->num_faces; i++) {

		i32 vertex_index = mesh->faces[i].v_idx*3;
		i32 material_id = mesh->material_ids[i/3];
		i32 tex_coord = mesh->faces[i].vt_idx*2;

		sh_vertex v = {
			.position = {.x = mesh->vertices[vertex_index], .y = mesh->vertices[vertex_index+1], .z = mesh->vertices[vertex_index+2]},
			.color = {
				materials[material_id].diffuse[0],
				materials[material_id].diffuse[1],
				materials[material_id].diffuse[2],
			},
			.tex = { .x = mesh->texcoords[tex_coord], .y = mesh->texcoords[tex_coord+1] },
			.material_id = material_id,
			.enable_material = color_from_material
		};

		buf_push(vs, v);
	}

	sh_vk_buffer_allocation_t vertices = sh_allocate_buffer(vk_ctx, sizeof(sh_vertex)*buf_len(vs), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	buf_push(mesh_buffers, vertices);

	i32 vert_count = mesh->num_faces;
	
	if(vk_ctx->mem->staging_buffer == NULL) {
		vk_ctx->mem->staging_buffer = (sh_vk_buffer_allocation_t*) calloc(1, sizeof(sh_vk_buffer_allocation_t));
		i32 staging_buffer_size = 1024*sizeof(sh_vertex);
		vk_ctx->mem->staging_buffer[0] = sh_allocate_buffer(vk_ctx, staging_buffer_size,  VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}

	sh_vk_buffer_allocation_t *stg_buf = vk_ctx->mem->staging_buffer;
	sh_vk_mapped_buffer_t mapped = sh_map_vk_buffer(vk_ctx, stg_buf, stg_buf->size);

	{
		u64 size_fit = (buf_len(vs)*sizeof(sh_vertex))/stg_buf->size;
		for(i32 i = 0; i < size_fit; i++) {
			sh_copy_data_to_mapped_buf(&mapped, stg_buf->size, (u8*)(vs) + i*stg_buf->size);
			sh_make_mapped_buf_visible(vk_ctx, &mapped, stg_buf->size);
			sh_copy_buf_to_buf_sync(vk_ctx, stg_buf, &vertices, stg_buf->size, i*stg_buf->size);
		}

		u64 remaining_size = buf_len(vs)*sizeof(sh_vertex) - size_fit*stg_buf->size;

		sh_copy_data_to_mapped_buf(&mapped, remaining_size, (u8*)(vs) + size_fit*stg_buf->size);
		sh_make_mapped_buf_visible(vk_ctx, &mapped, VK_WHOLE_SIZE);
		sh_copy_buf_to_buf_sync(vk_ctx, stg_buf, &vertices, remaining_size, size_fit*stg_buf->size);
	}

	sh_unmap_vk_buffer(vk_ctx, &mapped);

	return mesh_buffers;
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
		template.dstOffsets[1].x = (i32)( img->width/(pow(2, i)) );
		template.dstOffsets[1].y = (i32)( img->height/(pow(2, i)) );

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


typedef union sh_material_t{
	struct {
		sh_color_f32 diffuse;
		sh_color_f32 ambient;
		sh_color_f32 specular;
		sh_color_f32 pad;
	};
	f32 d[16];
} sh_material_t;

int main() {

	tinyobj_attrib_t car_mesh = {0};
	tinyobj_shape_t *car_shapes = NULL;
	u64 car_shape_count;
	tinyobj_material_t *car_materials = NULL;
	u64 car_material_count;

	tinyobj_attrib_t city_mesh = {0};
	tinyobj_shape_t *city_shapes;
	u64 city_shape_count;
	tinyobj_material_t *city_materials;
	u64 city_material_count;

	sh_material_t *car_mat = NULL;

	tinyobj_parse_obj(&car_mesh, &car_shapes, &car_shape_count, &car_materials, &car_material_count, "../assets/cw1/car.obj", sh_read_obj_file, NULL, TINYOBJ_FLAG_TRIANGULATE);
	tinyobj_parse_obj(&city_mesh, &city_shapes, &city_shape_count, &city_materials, &city_material_count, "../assets/cw1/city.obj", sh_read_obj_file, NULL, TINYOBJ_FLAG_TRIANGULATE);

	for(u32 i = 0; i < car_material_count; i++) {
		sh_material_t m = {
			.diffuse  = {car_materials[i].diffuse[0], car_materials[i].diffuse[1], car_materials[i].diffuse[2], 1.0f },
			.ambient  = {car_materials[i].ambient[0], car_materials[i].ambient[1], car_materials[i].ambient[2], 1.0f },
			.specular = {car_materials[i].specular[0], car_materials[i].specular[1], car_materials[i].specular[2], 1.0f },
		};
		buf_push(car_mat, m);
	}

	// POINTS(sharo): I changed the shader compiler from NO_PROFILE to CORE_PROFILE, check this if things are broken
	sh_shader_input_t shaders[] = {
		{ .filename = "./shader/vertex.vert", .type = SH_SHADER_TYPE_VERTEX },
		{ .filename = "./shader/fragment.frag", .type = SH_SHADER_TYPE_FRAGMENT },
	};

	sh_vk_spirv_shader_t *spirv_shaders = NULL;
	for(u32 i = 0; i < SH_ARRAY_SIZE(shaders); i++) {
		buf_push(spirv_shaders, sh_compile_shader_input(&shaders[i]));
	}

    // struct_type_check();
    sh_load_vulkan_funcs();

	sh_window_context_t *ctx = sh_win_setup();
	sh_vulkan_context_t *vk_ctx = sh_vk_setup(ctx, physical_device_selector, queue_family_selector, spirv_shaders);

	sh_camera cam;
	{
		sh_vec3 eye = {0.0f, 20.0f, 20.0f};
		sh_vec3 lookat = {0.0f, 0.0f, 0.0f};
		sh_vec3 up = {0.0, 1.0f, 0.0f};
		cam = sh_setup_camera(&eye, &lookat, &up);
	}

	sh_mat4 matrices[3] = {
		sh_perspective(90.0f, (f32)ctx->width/(f32)ctx->height, 0.1f, 100.0f),
		cam.mat, // view
		sh_make_mat4_scale(1.0f), // model
	};

	sh_mat4 *projection_mat = &matrices[0];
	sh_mat4_transpose(projection_mat);

	sh_vk_image_allocation_t *textures = NULL;
	sh_vk_image_view_allocation_t *texture_views = NULL;

	char tempbuf[256];
	u8 **imgs_to_free = NULL;
	i32 *texture_positions = NULL;

	for(u32 i = 0; i < city_material_count; i++) {

		if(city_materials[i].diffuse_texname == NULL) continue;
		buf_push(texture_positions, i);

		i32 width = 0;
		i32 height = 0;
		i32 channels = 4;

		sprintf(tempbuf, "../assets/cw1/%s", city_materials[i].diffuse_texname);
		unsigned char *data = stbi_load(tempbuf, &width, &height, &channels, 4);

		// TODO clean up the staging buffers and there should only be one
		sh_vk_image_allocation_t gpu_image = sh_allocate_2D_image(vk_ctx, width, height,

			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_FORMAT_R8G8B8A8_SRGB,
			1
		);

		sh_vk_buffer_allocation_t img_buffer = sh_allocate_buffer(vk_ctx, width*height*4,  VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		sh_vk_image_view_allocation_t image_view = sh_allocate_image_view(vk_ctx, &gpu_image, VK_IMAGE_ASPECT_COLOR_BIT);

		sh_copy_buf_data_to_gpu_sync(vk_ctx, &img_buffer, 0, width*height*4, (u8*)data);
		sh_copy_buf_to_img(vk_ctx, &img_buffer, &gpu_image);

		buf_push(textures, gpu_image);
		buf_push(texture_views, image_view);
		buf_push(imgs_to_free, data);

		// sh_deallocate_buffer(vk_ctx, &img_buffer);
	}


	for(u32 i = 0; i < buf_len(textures); i++) {
		sh_generate_img_mipmap(vk_ctx, textures + i);
	}

	for(u32 i = 0; i < buf_len(imgs_to_free); i++) {
		stbi_image_free(imgs_to_free[i]);
	}


	sh_vk_buffer_allocation_t *car_vertices = sh_make_buffer_from_obj_vertices(vk_ctx, &car_mesh, car_materials, car_material_count, 1);
	sh_vk_buffer_allocation_t *city_vertices = sh_make_buffer_from_obj_vertices(vk_ctx, &city_mesh, city_materials, city_material_count, 0);

	sh_vk_buffer_allocation_t uniform_buffer = sh_allocate_buffer(vk_ctx, sizeof(sh_mat4)*3, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	//TODO these should be parially bound why waste
	sh_vk_buffer_allocation_t material_buffer = sh_allocate_buffer(vk_ctx, sizeof(sh_material_t)*12, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	//TODO some sampler options and dynamic changes would be nice, please and thank you.
	sh_vk_sampler_allocation_t sampler = sh_allocate_sampler(vk_ctx, 1);

	i8 material_update = 1;
	i8 uniform_change = 1;
	i8 transfer_image_to_read = 1;

	//  Update the descriptor with the proper buffer info

	// TODO: replace with templated updates
	VkDescriptorBufferInfo buffer_info = {
		.buffer = uniform_buffer.buf,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	VkDescriptorBufferInfo *mat_buf = NULL;

	VkDescriptorBufferInfo x = {
		.buffer = material_buffer.buf,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	for(u32 i = 0; i < 12; i++) {
		x.offset = i*sizeof(sh_material_t);
		buf_push(mat_buf, x);
	}


	// VkDescriptorImageInfo sampler_info = {
	// 	.sampler = sampler.handle,
	// 	.imageView = VK_NULL_HANDLE,
	// };

		VkWriteDescriptorSet descriptor_writes[] = {
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = vk_ctx->descriptor_sets[0],
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = NULL,
			.pBufferInfo = &buffer_info,
			.pTexelBufferView = NULL
		},
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = vk_ctx->descriptor_sets[0],
			.dstBinding = 1,
			.dstArrayElement = 0,
			.descriptorCount = 12,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = NULL,
			.pBufferInfo = mat_buf,
			.pTexelBufferView = NULL
		},
	};



	SH_MARK_DEBUG_POINT(
		vkUpdateDescriptorSets(
			vk_ctx->device_info.ldevice,
			SH_ARRAY_SIZE(descriptor_writes),
			descriptor_writes,
			0,
			NULL
		)
	);


	VkDescriptorImageInfo *image_view_info = NULL;

	for(u32 i = 0; i < buf_len(texture_views); i++) {
		VkDescriptorImageInfo img_inf = {
			.sampler = sampler.handle,
			.imageView = texture_views[i].handle,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		buf_push(image_view_info, img_inf);

	}

	VkWriteDescriptorSet *bindless_textures = NULL;

	VkWriteDescriptorSet template = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = NULL,
		.dstSet = vk_ctx->descriptor_sets[0],
		.dstBinding = 2,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.pImageInfo = NULL,
		.pBufferInfo = NULL,
		.pTexelBufferView = NULL
	};

	for(u32 i = 0; i < buf_len(image_view_info); i++) {
		template.pImageInfo = image_view_info+i;
		template.dstArrayElement = texture_positions[i];
		buf_push(bindless_textures, template);
	}

	SH_MARK_DEBUG_POINT(
		vkUpdateDescriptorSets(
			vk_ctx->device_info.ldevice,
			buf_len(bindless_textures),
			bindless_textures,
			0,
			NULL
		)
	);

	VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkBuffer vertex_attrib_buffers[1] = {0};
	VkDeviceSize vertex_attrib_buf_offsets[1] = {0};

	f32 acc_time = ctx->time->time_sec;
	const f32 frame_time = 1.0f/60.0f;

    for(;!ctx->should_close;) {

		sh_handle_events(ctx);
		// handle changes

		// update at a fixed 60 FPS

		if(ctx->size_changed) {
			sh_recreate_rendering_resources(ctx, vk_ctx);
			matrices[0] = sh_perspective(90.0f, (f32)ctx->width/(f32)ctx->height, 0.1f, 100.0f);
			sh_mat4_transpose(matrices);
			uniform_change = 1;
		}

		while(acc_time >= frame_time) {

			f32 amount = 10.0f;

			if(ctx->shift.pressed) amount = 15.0f;
			if(ctx->ctrl.pressed) amount = 5.0f;

			if(ctx->keyboard['W'].pressed) {
				sh_camera_move_fwd(&cam, amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if(ctx->keyboard['S'].pressed) {
				sh_camera_move_fwd(&cam, -amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if(ctx->keyboard['D'].pressed) {
				sh_camera_move_hor(&cam, amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if(ctx->keyboard['A'].pressed) {
				sh_camera_move_hor(&cam, -amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if(ctx->keyboard[' '].pressed) {
				sh_camera_move_vert(&cam, amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if(ctx->keyboard['K'].pressed) {
				sh_camera_move_vert(&cam, -amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if((ctx->mouse->delta_x != 0 || ctx->mouse->delta_y != 0) && ctx->mouse->left.pressed) {
				sh_camera_lookat_mouse(&cam,
					(f32)ctx->mouse->delta_x*frame_time,
					(f32)ctx->mouse->delta_y*frame_time
				);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}
			acc_time -= frame_time;

			if(acc_time > 2*frame_time) {
				acc_time = 2*frame_time;
			}

		}

		acc_time += ctx->time->delta_time_sec;

		u32 image_view_index;
		CHECK_VK_RESULT(
			vkAcquireNextImageKHR(
				vk_ctx->device_info.ldevice, vk_ctx->swapchain, UINT64_MAX, vk_ctx->render_semaphore, VK_NULL_HANDLE,  &image_view_index
			)
		);

		// begin the frame

		VkCommandBuffer cmd_buf = vk_ctx->cmd_buffers[image_view_index];
		VkFramebuffer frame_buf = vk_ctx->framebuffers[image_view_index];

		vkResetCommandPool( vk_ctx->device_info.ldevice, vk_ctx->cmd_pool, 0);

		sh_begin_render(cmd_buf);

		if(material_update) {
			vkCmdUpdateBuffer(cmd_buf, material_buffer.buf, 0, sizeof(sh_material_t)*12, car_mat->d);
			sh_buf_mem_sync_copy(cmd_buf, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
			material_update = 0;
		}

		if(uniform_change) {
			vkCmdUpdateBuffer(cmd_buf, uniform_buffer.buf, 0, sizeof(sh_mat4)*3, matrices);
			sh_buf_mem_sync_copy(cmd_buf, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
			uniform_change = 0;
		}

		if(transfer_image_to_read) {

			for(u32 i = 0; i < buf_len(textures); i++) {
				sh_img_layout_transition(
					cmd_buf,
					&textures[i],

					VK_PIPELINE_STAGE_2_TRANSFER_BIT,
					VK_ACCESS_2_TRANSFER_WRITE_BIT,

					VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
					// TODO why is this supposed to be storage read and no
					
					// sampled_read
					VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					0, 1
				);

				sh_img_layout_transition(
					cmd_buf,
					&textures[i],

					VK_PIPELINE_STAGE_2_TRANSFER_BIT,
					VK_ACCESS_2_TRANSFER_WRITE_BIT,

					VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
					// TODO why is this supposed to be storage read and no

					// sampled_read
					VK_ACCESS_2_SHADER_STORAGE_READ_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					1, VK_REMAINING_MIP_LEVELS
				);
			}
			transfer_image_to_read = 0;
		}

		sh_begin_render_pass(ctx, vk_ctx, cmd_buf, frame_buf);

		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_ctx->pipeline);

		SH_MARK_DEBUG_POINT(
			vkCmdBindDescriptorSets (
				cmd_buf,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				vk_ctx->layout,
				0, 1,
				vk_ctx->descriptor_sets,
				0,
				NULL
			)
		);

		vertex_attrib_buffers[0] = city_vertices[0].buf;
		SH_MARK_DEBUG_POINT(vkCmdBindVertexBuffers2(cmd_buf, 0, SH_ARRAY_SIZE(vertex_attrib_buffers), vertex_attrib_buffers, vertex_attrib_buf_offsets, NULL, NULL));
		vkCmdDraw(cmd_buf, city_mesh.num_faces, 1, 0, 0);

		vertex_attrib_buffers[0] = car_vertices[0].buf;
		SH_MARK_DEBUG_POINT( vkCmdBindVertexBuffers2(cmd_buf, 0, SH_ARRAY_SIZE(vertex_attrib_buffers), vertex_attrib_buffers, vertex_attrib_buf_offsets, NULL, NULL) );
		vkCmdDraw(cmd_buf, car_mesh.num_faces, 1, 0, 0);

		sh_end_render_pass(cmd_buf);
		CHECK_VK_RESULT_MSG(vkEndCommandBuffer(cmd_buf), "We failed to finish the cmd recording");

		sh_queue_submit(
			vk_ctx->queue,
			1, &vk_ctx->render_semaphore, &wait_stages,
			1, &cmd_buf,
			1, &vk_ctx->present_semaphore, &wait_stages
		);

		VkPresentInfoKHR present = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.pNext = NULL,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &vk_ctx->present_semaphore,
			.swapchainCount = 1,
			.pImageIndices = &image_view_index,
			.pSwapchains = &vk_ctx->swapchain,
		};

		CHECK_VK_RESULT(vkQueuePresentKHR(vk_ctx->queue, &present));

		// highly inefficient and we are stalling every frame
        vkQueueWaitIdle(vk_ctx->queue);
    }

	destroy_vulkan_instance(vk_ctx);
	fflush(stdout);

	return 0;
}
