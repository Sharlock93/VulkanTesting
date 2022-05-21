#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 2) flat in int mat_id;
layout(location = 3) flat in int has_material;

layout(location = 0) out vec4 color;

layout(set = 0, binding = 2) uniform sampler2D samp[12];

void main() {
	if(has_material == 1) {
		color = fragColor;
	} else {
		color = vec4( texture(samp[(mat_id)], texCoord).rgb, 1.0f);
	}
}

