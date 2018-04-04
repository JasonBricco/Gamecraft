#version 440 core

in vec2 uv;
in vec3 vertColor;
out vec4 outColor;

uniform vec4 tint;
uniform sampler2D tex;

void main()
{
	outColor = texture(tex, uv) * tint; 
}
