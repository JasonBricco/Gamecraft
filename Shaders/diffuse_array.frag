#version 440 core

in vec3 uv;
in vec4 vertColor;
in float fogFactor;

out vec4 outColor;

uniform sampler2DArray tex;
uniform float ambient;
uniform vec3 fogColor;

void main()
{
	outColor = texture(tex, uv) * 1.5f;
	vec3 amb = vec3(ambient) * vertColor.rgb;
	outColor.xyz *= amb;
	outColor.xyz = mix(fogColor, outColor.xyz, fogFactor);
}
