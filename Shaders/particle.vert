#version 440 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 offset;

out vec2 uv;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	mat4 model = mat4(1.0);
	model[3] = model[0] * offset[0] + model[1] * offset[1] + model[2] * offset[2] + model[3];

	gl_Position = projection * view * model * vec4(pos, 1.0);
	uv = inUv;
}
