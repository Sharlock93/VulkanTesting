#pragma once

typedef union sh_color_i32 {
	struct {
		u8 red;
		u8 green;
		u8 blue;
		u8 alpha;
	};

	struct {
		u8 r;
		u8 g;
		u8 b;
		u8 a;
	};

	i32 hex;
} sh_color_i32;

typedef union sh_color_f32 {
	struct {
		f32 red;
		f32 green;
		f32 blue;
		f32 alpha;
	};

	struct {
		f32 r;
		f32 g;
		f32 b;
		f32 a;
	};

	f32 d[4];

} sh_color_f32;

typedef sh_color_f32 sh_vertex_color;

typedef struct sh_vertex {
	sh_vec3 position;
	sh_vertex_color color;
	sh_tex_coord tex;
	i32 material_id;
	i32 enable_material;
} sh_vertex;
