#if VERTEX

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 inUv;
layout (location = 2) in vec3 inColor;

out vec3 uv;
out vec3 vertColor;

uniform mat4 model0;
uniform mat4 view0;
uniform mat4 projection0;

void main()
{
	gl_Position = projection0 * view0 * model0 * vec4(pos, 1.0f);
	vertColor = inColor;
	uv = inUv;
}

#else

in vec3 uv;
in vec3 vertColor;

out vec4 outColor;

uniform vec4 ambient0;
uniform sampler2DArray tex;

void main()
{
	outColor = texture(tex, uv) * ambient0 * vec4(vertColor, 1.0f) * 1.5f;
}

#endif
