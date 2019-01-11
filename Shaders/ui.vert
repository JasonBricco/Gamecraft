#version 440 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec4 inColor;

uniform mat4 proj;

out vec2 uv;
out vec4 color;

void main()
{
	uv = inUv;
	color = inColor;
	gl_Position = proj * vec4(pos.xy, 0.0, 1.0);
}
