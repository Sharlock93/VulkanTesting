#version 450

vec2 pos[3] = vec2[](
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

vec4 color[3] = vec4[] (
		vec4(1., 0., 0., 1.),
		vec4(0., 1., 0., 1.),
		vec4(0., 0., 1., 1.)
);

layout(location = 0) out vec4 frag_color;

void main() {
	gl_Position  = vec4(pos[gl_VertexIndex], 0.0, 1.0);
	frag_color = color[gl_VertexIndex];
}
