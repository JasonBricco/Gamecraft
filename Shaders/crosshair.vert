#version 440 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 inUv;

out vec2 uv;

uniform mat4 model;
uniform mat4 projection;

void main()
{
	gl_Position = projection * model * vec4(pos, 1.0);
	uv = inUv;
}
