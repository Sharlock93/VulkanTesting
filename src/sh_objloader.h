#pragma once

#include "sh_tools.h"
#include "sh_types.h"
#include "sh_simple_vec_math.h"

typedef enum sh_obj_mtl_illum_e {
	SH_MTL_ILLUMINATION_COLOR_ON_AMB_OFF,
	SH_MTL_ILLUMINATION_COLOR_ON_AMB_ON,
	SH_MTL_ILLUMINATION_HIGHLIGHT,
	SH_MTL_ILLUMINATION_REFLECTION_ON_RTX_ON,
	SH_MTL_ILLUMINATION_TRANSPARANCY_ON
} sh_obj_mtl_illum_e;

typedef struct sh_obj_material_t {
	sh_color_f32 ambient;
	sh_color_f32 specular;
	sh_color_f32 diffuse;
	sh_color_f32 emission;

	f32 optical_density; // 0.001 - 10
	f32 dissolve;
	sh_obj_mtl_illum_e illum_type;
	const char *name;

} sh_obj_material_t;

typedef struct sh_obj_mesh_t {
	i32 vertex_count;
	i32 tex_coord_count;
	i32 normal_count;
	i32 face_count;

	sh_vec3 *vertices;
	sh_tex_coord *tex_coords;
	i32 *faces;

	i32 smoothing_group;
	const char *name;

} sh_obj_mesh_t;


sh_obj_mesh_t sh_read_obj_mesh(const char *file_name);
