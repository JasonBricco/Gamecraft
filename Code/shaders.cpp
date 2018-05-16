// Voxel Engine
// Jason Bricco

static bool ShaderHasErrors(GLuint handle, ShaderType type)
{
	int status = 0;
	GLint length = 0;

	if (type == SHADER_PROGRAM)
	{
		glGetProgramiv(handle, GL_LINK_STATUS, &status);

		if (status == GL_FALSE)
		{
			glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);

			GLchar* errorLog = Malloc(GLchar, length, "ShaderLink");
			glGetProgramInfoLog(handle, length, NULL, errorLog);
			
			OutputDebugString("Error! Shader program failed to link.");
			OutputDebugString(errorLog);
			Free(errorLog, "ShaderLink");
			return true;
		}
	}
	else
	{
		glGetShaderiv(handle, GL_COMPILE_STATUS, &status);

		if (status == GL_FALSE)
		{
			glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);

			GLchar* errorLog = Malloc(GLchar, length, "ShaderCompile");
			glGetShaderInfoLog(handle, length, NULL, errorLog);
			
			OutputDebugString("Error! Shader failed to compile.");
			OutputDebugString(errorLog);
			Free(errorLog, "ShaderCompile");
			return true;
		}
	}

	return false;
}

static GLuint LoadShader(char* path)
{
	char* code = ReadFileData(path);

	if (code == NULL)
	{
		OutputDebugString("Failed to load shader from file.");
		OutputDebugString(path);
		abort();
	}

	char* vertex[2] = { "#version 440 core\n#define VERTEX 1\n", code };
	char* frag[2] = { "#version 440 core\n", code };

	GLuint vS = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vS, 2, vertex, NULL);
	glCompileShader(vS);
	
	if (ShaderHasErrors(vS, VERTEX_SHADER))
	{
		OutputDebugString("Failed to compile the vertex shader.");
		abort();
	}

	GLuint fS = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fS, 2, frag, NULL);
	glCompileShader(fS);
	
	if (ShaderHasErrors(fS, FRAGMENT_SHADER))
	{
		OutputDebugString("Failed to compile the fragment shader.");
		abort();
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vS);
	glAttachShader(program, fS);
	glLinkProgram(program);
	
	if (ShaderHasErrors(program, SHADER_PROGRAM))
	{
		OutputDebugString("Failed to link the shaders into the program.");
		abort();
	}

	delete[] code;
	glDeleteShader(vS);
	glDeleteShader(fS);

	return program;
}

inline void UseShader(GLuint program)
{
	glUseProgram(program);
}

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
