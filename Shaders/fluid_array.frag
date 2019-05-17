#version 440 core

in vec3 uv;
in vec4 vertColor;
in float fogFactor;

out vec4 outColor;

uniform sampler2DArray tex;
uniform int animIndex;
uniform float ambient;
uniform vec3 fogColor;

void main()
{
	outColor = texture(tex, vec3(uv.x, uv.y, uv.z + animIndex));

    vec3 light = vertColor.rgb;
	float sun = vertColor.a;

	vec3 amb = vec3(ambient) * sun;
	amb = max(amb, 0.0666);
	amb = max(amb, light);

	outColor.xyz *= amb;
	outColor.xyz = mix(fogColor, outColor.xyz, fogFactor);
	outColor.a = 0.5;
}
