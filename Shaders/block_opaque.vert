#version 440 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 inUv;
layout (location = 2) in vec4 inColor;

out vec3 uv;
out vec4 vertColor;
out float fogFactor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float fogStart;
uniform float fogEnd;

void main()
{
	gl_Position = projection * view * model * vec4(pos, 1.0);
	vertColor = inColor;
	uv = inUv;

	float dist = length(gl_Position.xyz);
	fogFactor = clamp((fogEnd - dist) / (fogEnd - fogStart), 0.0, 1.0);
}
