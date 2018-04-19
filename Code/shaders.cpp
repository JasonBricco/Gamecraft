// Voxel Engine
// Jason Bricco

static char* ShaderFromFile(char* fileName)
{
	char* path = PathToAsset(fileName);
	char* buffer = NULL;

	ifstream file(path);

	if (file)
	{
		file.seekg(0, file.end);
		uint32_t length = (uint32_t)file.tellg() + 1;
		file.seekg(0, file.beg);

		char* inputBuffer = new char[length];
		memset(inputBuffer, 0, length);
		file.read(inputBuffer, length);
		inputBuffer[length - 1] = 0;

		if (inputBuffer) file.close();
		else
		{
			LogError("Failed to read shader file!");
			file.close();
			delete[] inputBuffer;
			return NULL;
		}

		buffer = inputBuffer;
		inputBuffer = NULL;
	}
	else
	{
		LogError("Could not find the shader: ");
		LogError(path);
		return NULL;
	}

	return buffer;
}

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

			GLchar* errorLog = (GLchar*)malloc(length);
			glGetProgramInfoLog(handle, length, NULL, errorLog);
			
			LogError("Error! Shader program failed to link.");
			LogError(errorLog);
			free(errorLog);
			return true;
		}
	}
	else
	{
		glGetShaderiv(handle, GL_COMPILE_STATUS, &status);

		if (status == GL_FALSE)
		{
			glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);

			GLchar* errorLog = (GLchar*)malloc(length);
			glGetShaderInfoLog(handle, length, NULL, errorLog);
			
			LogError("Error! Shader failed to compile.");
			LogError(errorLog);
			free(errorLog);
			return true;
		}
	}

	return false;
}

static GLuint LoadShaders(char* vertexPath, char* fragPath)
{
	char* vertex = ShaderFromFile(vertexPath);

	if (vertex == NULL)
	{
		LogError("Failed to load vertex shader from file.");
		abort();
	}

	GLuint vS = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vS, 1, &vertex, NULL);
	glCompileShader(vS);
	
	if (ShaderHasErrors(vS, VERTEX_SHADER))
	{
		LogError("Failed to compile the vertex shader.");
		abort();
	}

	char* frag = ShaderFromFile(fragPath);

	if (frag == NULL)
	{
		LogError("Failed to load fragment shader from file.");
		abort();
	}

	GLuint fS = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fS, 1, &frag, NULL);
	glCompileShader(fS);
	
	if (ShaderHasErrors(fS, FRAGMENT_SHADER))
	{
		LogError("Failed to compile the fragment shader.");
		abort();
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vS);
	glAttachShader(program, fS);
	glLinkProgram(program);
	
	if (ShaderHasErrors(program, SHADER_PROGRAM))
	{
		LogError("Failed to link the shaders into the program.");
		abort();
	}

	free(vertex);
	free(frag);
	glDeleteShader(vS);
	glDeleteShader(fS);

	return program;
}

inline void UseShader(GLuint program)
{
	glUseProgram(program);
}

static GLint GetUniformLocation(GLint program, GLchar* name)
{
	unordered_map<string, GLint>::iterator it = g_renderer.uniforms.find(name);

	if (it == g_renderer.uniforms.end())
		g_renderer.uniforms[name] = glGetUniformLocation(program, name);
	
	return g_renderer.uniforms[name];
}

inline void SetUniformSampler(GLint program, GLchar* name, GLint tex)
{
	glActiveTexture(GL_TEXTURE0 + tex);
	GLint loc = GetUniformLocation(program, name);
	glUniform1i(loc, tex);
}

inline void SetUniform(int ID, GLchar* name, GLfloat f)
{
	GLint loc = GetUniformLocation(g_renderer.programs[ID], name);
	glUniform1f(loc, f);
}

inline void SetUniform(int ID, GLchar* name, vec2 v)
{
	GLint loc = GetUniformLocation(g_renderer.programs[ID], name);
	glUniform2f(loc, v.x, v.y);
}

inline void SetUniform(int ID, GLchar* name, vec3 v)
{
	GLint loc = GetUniformLocation(g_renderer.programs[ID], name);
	glUniform3f(loc, v.x, v.y, v.z);
}

inline void SetUniform(int ID, GLchar* name, vec4 v)
{
	GLint loc = GetUniformLocation(g_renderer.programs[ID], name);
	glUniform4f(loc, v.x, v.y, v.z, v.w);
}

inline void SetUniform(int ID, GLchar* name, mat4 m)
{
	GLint loc = GetUniformLocation(g_renderer.programs[ID], name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(m));
}
