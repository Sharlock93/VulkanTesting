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
#include "sh_vk_enum_strings.c"
#include "sh_load_vulkan.c"
#include "sh_vulkan_mem_allocator.c"
#include "sh_setup_vulkan.c"
#include "sh_objloader.c"
#include "sh_vk_tools.c"
#include "sh_engine_core.c"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobject/tinyobj_loader_c.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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
			.normal = {.x = mesh->normals[vertex_index], .y = mesh->normals[vertex_index+1], .z = mesh->normals[vertex_index+2]},
			.color = {
				(u8)(materials[material_id].diffuse[0]*255.0f),
				(u8)(materials[material_id].diffuse[1]*255.0f),
				(u8)(materials[material_id].diffuse[2]*255.0f),
				255,
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

typedef union sh_material_t{
	struct {
		union {
			sh_color_f32 diffuse;
			struct {
				f32 diff_a;
				f32 diff_b;
				f32 diff_c;
				f32 roughness; // don't assign to 
			};
		};

		union {
			sh_color_f32 ambient;
			struct {
				struct {
					f32 amb_a;
					f32 amb_b;
					f32 amb_c;
					f32 metallic;
				};
			};
		};

		union {
			sh_color_f32 specular;
			struct {
				f32 spec_a;
				f32 spec_b;
				f32 spec_c;
				f32 sheen;
			};
		};

		sh_color_f32 emission;
	};
	f32 d[4+4+4+4];
} sh_material_t;

typedef struct sh_light_t {
	sh_vec4 light_pos;
	sh_vec4 light_color;
	sh_vec4 pad1;
	sh_vec4 pad2;
} sh_light_t;

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

SH_MAKE_FCHANGE_CALLBACK(shader_changed) {

	sh_ptr *a = (sh_ptr*)args;

	
	sh_vk_shader_module_t *shader = (sh_vk_shader_module_t*)a[0];
	sh_window_context_t  *ctx = (sh_window_context_t*)(a[1]);
	sh_vulkan_context_t  *vk_ctx = (sh_vulkan_context_t*)(a[2]);

	sh_shader_input_t new_shader_input = {
		.filename = file,
		.type = 0
	};

	sh_shader_type_t type;
	switch(shader->stage) {
		case VK_SHADER_STAGE_VERTEX_BIT:
			type = SH_SHADER_TYPE_VERTEX;
			break;
		case VK_SHADER_STAGE_FRAGMENT_BIT:
			type = SH_SHADER_TYPE_FRAGMENT;
			break;
		default:
			SH_ASSERT_EXIT(false, "Shader Stage invalid");
			exit(1);
	}

	new_shader_input.type = type;

	sh_vk_spirv_shader_t new_shader = sh_compile_shader_input(&new_shader_input);

	if(new_shader.size > 0) {
		vkDestroyShaderModule(vk_ctx->device_info.ldevice, shader->shader_module, NULL);

		VkShaderModuleCreateInfo shader_module_create_info = {
			.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.pNext = NULL,
			.flags = 0,
			.codeSize = new_shader.size,
			.pCode = new_shader.data
		};

		CHECK_VK_RESULT(
			vkCreateShaderModule(
				vk_ctx->device_info.ldevice,
				&shader_module_create_info,
				NULL,
				&shader->shader_module	
			)
		);

		// shader[0]->data = new_shader.data;
		// shader[0]->size = new_shader.size;
		ctx->size_changed = 1;

		return 1;
	}

	return 0;
}

SH_MAKE_FCHANGE_CALLBACK(shader_meow) {

	printf("%s\n", file);

	return 0;
}


int main() {

	tinyobj_attrib_t material_test = {0};
	tinyobj_shape_t *material_shape = NULL;
	u64 material_shape_count;
	tinyobj_material_t *material_materials = NULL;
	u64 material_materials_count;

	SH_BUFFER_TYPE(sh_material_t*) materials_list = NULL;

	tinyobj_parse_obj(
		&material_test,
		&material_shape,
		&material_shape_count,
		&material_materials,
		&material_materials_count,
		"S:/code/projects/vulkan/assets/cw2/materialtest.obj",
		sh_read_obj_file,
		NULL,
		TINYOBJ_FLAG_TRIANGULATE
	);
	// tinyobj_parse_obj(&city_mesh, &city_shapes, &city_shape_count, &city_materials, &city_material_count, "../assets/cw2/city.obj", sh_read_obj_file, NULL, TINYOBJ_FLAG_TRIANGULATE);

	log_debugl("%d", material_materials_count);

	for(u32 i = 0; i < material_materials_count; i++) {
		sh_material_t m = {
			.diffuse  = { material_materials[i].diffuse[0],  material_materials[i].diffuse[1],  material_materials[i].diffuse[2], material_materials[i].roughness },
			.ambient  = { material_materials[i].ambient[0],  material_materials[i].ambient[1],  material_materials[i].ambient[2], material_materials[i].metallic },
			.specular = { material_materials[i].specular[0], material_materials[i].specular[1], material_materials[i].specular[2], material_materials[i].sheen },
			.emission = { material_materials[i].emission[0], material_materials[i].emission[1], material_materials[i].emission[2], 0.0f },
		};
		buf_push(materials_list, m);
	}

	// POINTS(sharo): I changed the shader compiler from NO_PROFILE to CORE_PROFILE, check this if things are broken
	sh_shader_input_t shaders[] = {
		{ .filename = "./shader/vertex.vert", .type = SH_SHADER_TYPE_VERTEX },
		{ .filename = "./shader/fragment.frag", .type = SH_SHADER_TYPE_FRAGMENT },

		{ .filename = "./shader/vertex_shader_post_process.vert", .type = SH_SHADER_TYPE_VERTEX },
		{ .filename = "./shader/fragment_shader_post_process.frag", .type = SH_SHADER_TYPE_FRAGMENT },

	};

	sh_file_tracker_t tracker = sh_create_file_tracker();

	sh_vk_spirv_shader_t *spirv_shaders = NULL;
	for(u32 i = 0; i < SH_ARRAY_SIZE(shaders); i++) {
		buf_push(spirv_shaders, sh_compile_shader_input(&shaders[i]));
	}

    // struct_type_check();
    sh_load_vulkan_funcs();


	sh_window_context_t *ctx = sh_win_setup();
	sh_vulkan_context_t *vk_ctx = sh_vk_setup(ctx, physical_device_selector, queue_family_selector, spirv_shaders);

	// for(u32 i = 0; i < buf_len(vk_ctx->shader_modules); i++) {
	// 	sh_ptr *args = NULL;
	// 	buf_push(args, (sh_ptr)&(vk_ctx->shader_modules[i]));
	// 	buf_push(args, (sh_ptr)ctx);
	// 	buf_push(args, (sh_ptr)vk_ctx);
	// 	sh_file_tracker_add(&tracker, shaders[i].filename, shader_changed, args);
	// }

	sh_camera cam;
	{
		sh_vec3 eye = {0.0f, 20.0f, 20.0f};
		sh_vec3 lookat = {0.0f, 0.0f, 0.0f};
		sh_vec3 up = {0.0, 1.0f, 0.0f};
		cam = sh_setup_cam(&eye, &lookat, &up);
	}

	sh_mat4 matrices[3] = {
		sh_perspective(50.0f, (f32)ctx->width/(f32)ctx->height, 0.1f, 100.0f),
		cam.mat, // view
		sh_make_mat4_scale(1.0f), // model
	};

	sh_mat4 *projection_mat = &matrices[0];
	sh_mat4_transpose(projection_mat);

	sh_light_t *lights = NULL;
	buf_fit(lights, 1);

	buf_push(lights, (sh_light_t){ .light_pos = {-10, 9.3f, -3, 1.0f}, .light_color = { SH_HEX_TO_FLOAT4(0xe9bacc00) } });
	buf_push(lights, (sh_light_t){ .light_pos = { 10, 9.3f, -3, 1.0f}, .light_color = { 0.71f, 0.2f, 0.76f, 1.0f} });
	buf_push(lights, (sh_light_t){ .light_pos = {  0, 9.3f, -3, 1.0f}, .light_color = { 0.16f, 0.78f, 0.74f, 1.0f } });
	buf_push(lights, (sh_light_t){ .light_pos = {  0, 5.3f, 0, 1.0f},  .light_color = { .5f, 0.2f, 0.3f, 1.0f }});

	u32 light_count = buf_len(lights);

	i32 uniform_buffer_size = sizeof(sh_mat4)*3 + sizeof(sh_vec3) + sizeof(i32) + sizeof(f32) + sizeof(i32)*2;
	sh_vk_buffer_allocation_t uniform_buffer = sh_allocate_buffer( vk_ctx, uniform_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	sh_vk_buffer_allocation_t *material_vertices = sh_make_buffer_from_obj_vertices(vk_ctx, &material_test, material_materials, material_shape_count, 1);
	sh_vk_buffer_allocation_t light_buffer   = sh_allocate_buffer( vk_ctx, sizeof(sh_light_t)*light_count, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	u32 indices[] = {
		0, 1, 2,
		0, 2, 3
	};

	sh_vk_buffer_allocation_t index_buffer = sh_allocate_buffer( vk_ctx, sizeof(u32)*6, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	//TODO these should be parially bound why waste
	sh_vk_buffer_allocation_t material_buffer = sh_allocate_buffer( vk_ctx, sizeof(sh_material_t)*34, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	//TODO some sampler options and dynamic changes would be nice, please and thank you.
	sh_vk_sampler_allocation_t sampler = sh_allocate_sampler(vk_ctx, 0);

	i8 material_update = 1;
	i8 uniform_change = 1;
	i8 light_change = 1;
	i8 transfer_image_to_read = 1;

	//  Update the descriptor with the proper buffer info
	// TODO: replace with templated updates
	VkDescriptorBufferInfo buffer_info = {
		.buffer = uniform_buffer.buf,
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	VkDescriptorBufferInfo *light_buf = NULL;
	VkDescriptorBufferInfo *mat_buf = NULL;

	VkDescriptorBufferInfo x = {
		.offset = 0,
		.range = VK_WHOLE_SIZE
	};

	x.buffer = light_buffer.buf;
	for(u32  i = 0; i < light_count; i++) {
		x.offset = i*sizeof(sh_light_t);
		buf_push(light_buf, x);
	}

	x.buffer = material_buffer.buf;
	for(u32 i = 0; i < 34; i++) {
		x.offset = i*sizeof(sh_material_t);
		buf_push(mat_buf, x);
	}

	
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
			.descriptorCount = 34,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = NULL,
			.pBufferInfo = mat_buf,
			.pTexelBufferView = NULL
		},
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = vk_ctx->descriptor_sets[0],
			.dstBinding = 3,
			.dstArrayElement = 0,
			.descriptorCount = light_count,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = NULL,
			.pBufferInfo = light_buf,
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

	VkDescriptorImageInfo g_buffer_descriptors[] = {
		{ .sampler = sampler.handle, .imageView = vk_ctx->g_buffer[0].handle, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
		{ .sampler = sampler.handle, .imageView = vk_ctx->g_buffer[1].handle, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL },
		{ .sampler = sampler.handle, .imageView = vk_ctx->g_buffer[2].handle, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
		{ .sampler = sampler.handle, .imageView = vk_ctx->g_buffer[3].handle, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
	};

	VkWriteDescriptorSet descriptor_write_textures[] = {

		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = vk_ctx->descriptor_sets[0],
			.dstBinding = 2,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = &g_buffer_descriptors[0],
			.pBufferInfo = 0,
			.pTexelBufferView = NULL
		},
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = vk_ctx->descriptor_sets[0],
			.dstBinding = 2,
			.dstArrayElement = 1,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = g_buffer_descriptors+1,
			.pBufferInfo = 0,
			.pTexelBufferView = NULL
		},
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = vk_ctx->descriptor_sets[0],
			.dstBinding = 2,
			.dstArrayElement = 2,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = g_buffer_descriptors+2,
			.pBufferInfo = 0,
			.pTexelBufferView = NULL
		},
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = vk_ctx->descriptor_sets[0],
			.dstBinding = 2,
			.dstArrayElement = 3,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = g_buffer_descriptors+3,
			.pBufferInfo = 0,
			.pTexelBufferView = NULL
		},

	};

	SH_MARK_DEBUG_POINT(
		vkUpdateDescriptorSets(
			vk_ctx->device_info.ldevice,
			SH_ARRAY_SIZE(descriptor_write_textures),
			descriptor_write_textures,
			0,
			NULL
		)
	);

	VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkBuffer vertex_attrib_buffers[1] = {0};
	VkDeviceSize vertex_attrib_buf_offsets[1] = {0};

	f32 acc_time = ctx->time->time_sec;
	const f32 frame_time = 1.0f/60.0f;


	i8 animate_light = 0;

	u32 image_view_index = 0;

	for(;!ctx->should_close;) {

		sh_handle_events(ctx);
		// handle changes

		sh_file_tracker_update(&tracker);
		// update at a fixed 60 FPS

		if(ctx->size_changed) {
			log_debugl("Sorry I haven't implemented resize yet");
			exit(1);
			sh_recreate_rendering_resources(ctx, vk_ctx);
			matrices[0] = sh_perspective(50.0f, (f32)ctx->width/(f32)ctx->height, 0.1f, 100.0f);
			sh_mat4_transpose(matrices);
			uniform_change = 1;
		}

		while(acc_time >= frame_time) {

			f32 amount = 10.0f;

			if(ctx->shift.pressed) amount = 15.0f;
			if(ctx->ctrl.pressed) amount = 5.0f;

			if(ctx->keyboard['W'].pressed) {
				sh_cam_move_fwd(&cam, amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if(ctx->keyboard['S'].pressed) {
				sh_cam_move_fwd(&cam, -amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if(ctx->keyboard['D'].pressed) {
				sh_cam_move_hor(&cam, amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if(ctx->keyboard['A'].pressed) {
				sh_cam_move_hor(&cam, -amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if(ctx->keyboard[' '].pressed) {
				sh_cam_move_vert(&cam, amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if(ctx->keyboard['K'].pressed) {
				sh_cam_move_vert(&cam, -amount*frame_time);
				matrices[1] = cam.mat;
				uniform_change = 1;
			}

			if((ctx->mouse->delta_x != 0 || ctx->mouse->delta_y != 0) && ctx->mouse->left.pressed) {
				sh_cam_lookat_mouse(&cam,
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

			static i32 angel = 0;

			if(animate_light) {
				lights[0].light_pos.x = 10*SH_COSFD(angel);
				lights[1].light_pos.y = 10*SH_SINFD(angel);
				lights[2].light_pos.z = 10*SH_COSFD(angel);
				angel++;
				light_change = 1;
			}
		}

		if(ctx->keyboard['L'].pressed_once) {
			animate_light = !animate_light;
		}

		if(ctx->keyboard['0'].pressed_once) { light_count = 0; uniform_change = 1; }
		if(ctx->keyboard['1'].pressed_once) { light_count = 1; uniform_change = 1; }
		if(ctx->keyboard['2'].pressed_once) { light_count = 2; uniform_change = 1; }
		if(ctx->keyboard['3'].pressed_once) { light_count = 3; uniform_change = 1; }
		if(ctx->keyboard['4'].pressed_once) { light_count = 4; uniform_change = 1; }

		acc_time += ctx->time->delta_time_sec;
		CHECK_VK_RESULT(
			vkAcquireNextImageKHR(
				vk_ctx->device_info.ldevice, vk_ctx->swapchain, UINT64_MAX, vk_ctx->render_semaphore, VK_NULL_HANDLE,  &image_view_index
			)
		);

		// begin the frame

		VkCommandBuffer cmd_buf = vk_ctx->cmd_buffers[image_view_index];
		VkFramebuffer frame_buf = vk_ctx->framebuffers[image_view_index];

		// reset only when we hit the final image
		if(image_view_index >= (buf_len(vk_ctx->cmd_buffers)-1)) {
			vkQueueWaitIdle(vk_ctx->queue);
			SH_MARK_DEBUG_POINT(vkResetCommandPool( vk_ctx->device_info.ldevice, vk_ctx->cmd_pool, 0));
		}

		sh_begin_render(cmd_buf);

		if(material_update) {
			vkCmdUpdateBuffer(cmd_buf, material_buffer.buf, 0, sizeof(sh_material_t)*buf_len(materials_list), materials_list->d);
			sh_buf_mem_sync_copy(cmd_buf, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
			material_update = 0;
		}

		if(uniform_change) {
			vkCmdUpdateBuffer(cmd_buf, uniform_buffer.buf, 0,                                             sizeof(sh_mat4)*3, matrices);
			vkCmdUpdateBuffer(cmd_buf, uniform_buffer.buf, sizeof(sh_mat4)*3,                             sizeof(sh_vec3),   &cam.eye);
			vkCmdUpdateBuffer(cmd_buf, uniform_buffer.buf, sizeof(sh_mat4)*3+sizeof(sh_vec3),             sizeof(i32),       &light_count);

			vkCmdUpdateBuffer(cmd_buf, uniform_buffer.buf, sizeof(sh_mat4)*3+sizeof(sh_vec3)+sizeof(i32)+sizeof(f32), sizeof(u32), &ctx->width);
			vkCmdUpdateBuffer(cmd_buf, uniform_buffer.buf, sizeof(sh_mat4)*3+sizeof(sh_vec3)+sizeof(i32)+sizeof(f32)+sizeof(u32), sizeof(u32), &ctx->height);
			sh_buf_mem_sync_copy(cmd_buf, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
			uniform_change = 0;
		}

		vkCmdUpdateBuffer(cmd_buf, uniform_buffer.buf, sizeof(sh_mat4)*3+sizeof(sh_vec3)+sizeof(i32), sizeof(f32), &ctx->time->time_sec);
		sh_buf_mem_sync_copy(cmd_buf, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);

		if(light_change) {
			vkCmdUpdateBuffer(cmd_buf, light_buffer.buf, 0, sizeof(sh_light_t)*light_count, lights);
			sh_buf_mem_sync_copy(cmd_buf, VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
			light_change = 0;
		}

		if(transfer_image_to_read) {
			vkCmdUpdateBuffer(cmd_buf, index_buffer.buf, 0, sizeof(u32)*6, indices);
			sh_buf_mem_sync_copy(cmd_buf, VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, VK_ACCESS_2_INDEX_READ_BIT);
			transfer_image_to_read = 0;
		}

		VkClearValue clear_vals[] = {
			{ .color = { vk_ctx->clear_color[0], vk_ctx->clear_color[1], vk_ctx->clear_color[2], vk_ctx->clear_color[3], } },
			{ .depthStencil = { .depth = 1.0f } },
			{ .color = { vk_ctx->clear_color[0], vk_ctx->clear_color[1], vk_ctx->clear_color[2], vk_ctx->clear_color[3], } },
			{ .color = { vk_ctx->clear_color[0], vk_ctx->clear_color[1], vk_ctx->clear_color[2], vk_ctx->clear_color[3], } },
		};


		// g buffer build
		sh_begin_render_pass(ctx, vk_ctx, cmd_buf, frame_buf, vk_ctx->render_pass[0], SH_ARRAY_SIZE(clear_vals), clear_vals );
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_ctx->pipeline[0]);

		SH_MARK_DEBUG_POINT(
			vkCmdBindDescriptorSets (
				cmd_buf,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				vk_ctx->layout[0],
				0, 1,
				vk_ctx->descriptor_sets,
				0,
				NULL
			)
		);

		vertex_attrib_buffers[0] = material_vertices->buf;
		SH_MARK_DEBUG_POINT(vkCmdBindVertexBuffers2(cmd_buf, 0, 1, vertex_attrib_buffers, vertex_attrib_buf_offsets, NULL, NULL));
		SH_MARK_DEBUG_POINT(vkCmdDraw(cmd_buf, material_test.num_faces, 1, 0, 0));

		sh_end_render_pass(cmd_buf);

		sh_img_layout_transition(
			cmd_buf,
			&vk_ctx->g_buffer_img[0],
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,

			VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
			VK_ACCESS_2_SHADER_STORAGE_READ_BIT,

			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,

			VK_IMAGE_ASPECT_COLOR_BIT,

			0,
			1
			);

		sh_img_layout_transition(
			cmd_buf,
			&vk_ctx->g_buffer_img[1],
			VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
			VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,

			VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
			VK_ACCESS_2_SHADER_STORAGE_READ_BIT,

			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,

			VK_IMAGE_ASPECT_DEPTH_BIT,
			0,
			1
		);

		sh_img_layout_transition(
			cmd_buf,
			&vk_ctx->g_buffer_img[2],
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,

			VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
			VK_ACCESS_2_SHADER_STORAGE_READ_BIT,

			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,

			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1
		);


		sh_img_layout_transition(
			cmd_buf,
			&vk_ctx->g_buffer_img[3],
			VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,

			VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
			VK_ACCESS_2_SHADER_STORAGE_READ_BIT,

			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,

			VK_IMAGE_ASPECT_COLOR_BIT,

			0,
			1
		);

		// SH_MARK_DEBUG_POINT(vkCmdPipelineBarrier2(cmd_buf, &dep_info));
		// shading pass

		VkClearValue shading_pass_clear[] = {
			{ .color = { vk_ctx->clear_color[0], vk_ctx->clear_color[1], vk_ctx->clear_color[2], vk_ctx->clear_color[3], } },
		};

		VkFramebuffer shading_frame_buf = vk_ctx->framebuffers[image_view_index+3];
		sh_begin_render_pass(ctx, vk_ctx, cmd_buf, shading_frame_buf, vk_ctx->render_pass[1], 1, shading_pass_clear);
		vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_ctx->pipeline[1]);

		SH_MARK_DEBUG_POINT(
			vkCmdBindDescriptorSets (
				cmd_buf,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				vk_ctx->layout[0],
				0, 1,
				vk_ctx->descriptor_sets,
				0,
				NULL
			)
		);

		SH_MARK_DEBUG_POINT(vkCmdBindIndexBuffer(cmd_buf, index_buffer.buf, 0, VK_INDEX_TYPE_UINT32));
		SH_MARK_DEBUG_POINT(vkCmdDrawIndexed(cmd_buf, 6, 1, 0, 0, 0));

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

		// image_view_index = (image_view_index+1)%3;
		CHECK_VK_RESULT(vkQueuePresentKHR(vk_ctx->queue, &present));
		// exit(1);
	}

	// destroy_vulkan_instance(vk_ctx);
	fflush(stdout);

	return 0;
}
