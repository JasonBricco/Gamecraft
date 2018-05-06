// Voxel Engine
// Jason Bricco

enum ShaderType
{
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	SHADER_PROGRAM
};

static char* ShaderFromFile(char* fileName);
static bool ShaderHasErrors(GLuint handle, ShaderType type);

static GLuint LoadShader(char* path);
inline void UseShader(GLuint program);

static GLint GetUniformLocation(Renderer* rend, GLint program, GLchar* name);

inline void SetUniform(Renderer* rend, int ID, GLchar* name, GLfloat f);
inline void SetUniform(Renderer* rend, int ID, GLchar* name, Vec2 v);
inline void SetUniform(Renderer* rend, int ID, GLchar* name, Vec3 v);
inline void SetUniform(Renderer* rend, int ID, GLchar* name, Vec4 v);
inline void SetUniform(Renderer* rend, int ID, GLchar* name, Matrix4 m);
