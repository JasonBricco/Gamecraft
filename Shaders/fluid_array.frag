#version 440 core

in vec3 uv;
in vec4 vertColor;
in float fogFactor;
in flat int lowIndex;
in flat int highIndex;
in flat float blendFactor;

out vec4 outColor;

uniform sampler2DArray tex;
uniform float ambient;
uniform vec3 fogColor;

void main()
{
	vec4 lower = texture(tex, vec3(uv.x, uv.y, uv.z + lowIndex));
	vec4 upper = texture(tex, vec3(uv.x, uv.y, uv.z + highIndex));

    outColor = mix(lower, upper, blendFactor);
	vec3 amb = vec3(ambient) * vertColor.rgb;
	outColor.xyz *= amb;
    outColor.xyz = mix(fogColor, outColor.xyz, fogFactor);
    outColor.a = vertColor.a;
}
