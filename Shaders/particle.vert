#version 440 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 inUv;
layout (location = 2) in mat4 model;

out vec2 uv;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	mat4 mv = view * model;

	mv[0][0] = mv[2][2] = 1;
	mv[0][1] = mv[0][2] = mv[2][0] = mv[2][1] = 0;

	gl_Position = projection * mv * vec4(pos, 1.0);
	uv = inUv.xy;
}
