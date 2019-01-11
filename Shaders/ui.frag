#version 440 core

in vec2 uv;
in vec4 color;

out vec4 outColor;

uniform sampler2D tex;

void main()
{
	outColor = color * texture(tex, uv);
}
