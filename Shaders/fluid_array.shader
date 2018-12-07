#if VERTEX

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 inUv;
layout (location = 2) in vec4 inColor;

out vec3 uv;
out vec4 vertColor;

uniform float time;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0f);
    vertColor = inColor;
    uv = inUv;
    uv.x += time * 0.2f;
}

#else

in vec3 uv;
in vec4 vertColor;

out vec4 outColor;

uniform sampler2DArray tex;
uniform float ambient;

void main()
{
    outColor = texture(tex, uv) * 1.5f;

	vec3 light = vertColor.rgb;
	float sun = vertColor.a;

	vec3 amb = vec3(ambient) * sun;
	amb = max(amb, light);

	outColor.xyz *= amb;
    outColor.a = 0.5f;
}

#endif
