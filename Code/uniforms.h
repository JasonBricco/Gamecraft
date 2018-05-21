// Voxel Engine
// Jason Bricco

inline void SetUniform(GLint loc, GLfloat f)
{
    glUniform1f(loc, f);
}

inline void SetUniform(GLint loc, vec2 v)
{
    glUniform2f(loc, v.x, v.y);
}

inline void SetUniform(GLint loc, vec3 v)
{
    glUniform3f(loc, v.x, v.y, v.z);
}

inline void SetUniform(GLint loc, vec4 v)
{
    glUniform4f(loc, v.x, v.y, v.z, v.w);
}

inline void SetUniform(GLint loc, mat4 m)
{
    glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(m));
}
