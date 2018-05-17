#if VERTEX

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 inUv;
layout (location = 2) in vec3 inColor;

out vec3 uv;
out vec3 vertColor;

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
in vec3 vertColor;

out vec4 outColor;

uniform sampler2DArray tex;

void main()
{
    outColor = texture(tex, uv) * vec4(vertColor, 1.0f) * 1.5f;
    outColor.a = 0.5f;
}

#endif
