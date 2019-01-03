#version 440 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 offset;

out vec2 uv;

uniform mat4 view;
uniform mat4 projection;

mat4 translateOffset()
{
    return mat4(vec4(1.0, 0.0, 0.0, 0.0),
        		vec4(0.0, 1.0, 0.0, 0.0),
        		vec4(0.0, 0.0, 1.0, 0.0),
        		vec4(offset.x, offset.y, offset.z, 1.0));
}

void main()
{
	mat4 model = translateOffset();
	gl_Position = projection * view * model * vec4(pos, 1.0);
	uv = inUv;
}
