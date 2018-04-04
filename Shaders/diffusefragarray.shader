#version 440 core

in vec3 uv;
in vec3 vertColor;

out vec4 outColor;

uniform vec4 ambient;
uniform sampler2DArray tex;

void main()
{
	outColor = texture(tex, uv) * ambient * 1.75f;
}
