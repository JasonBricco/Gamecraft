#if VERTEX

layout (location = 0) in vec2 pos;

void main()
{
	gl_Position = vec4(pos, 0.0f, 1.0f);
}

#else

out vec4 outColor;
uniform vec4 inColor;

void main()
{
	outColor = inColor;
}

#endif
