// Voxel Engine
// Jason Bricco

static vec3 g_worldUp = vec3(0.0f, 1.0f, 0.0f);
static int g_windowWidth = 1024;
static int g_windowHeight = 768;

static unordered_map<string, GLint> g_uniformMap;
static int g_paramCount = 12;

static void InitializeMesh(Mesh* mesh)
{
	glGenVertexArrays(1, &mesh->va);
	glBindVertexArray(mesh->va);

	// Vertex position attribute buffer.
	glGenBuffers(1, &mesh->vb);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, g_paramCount * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(0);

	// Texture coordinates (UVs).
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, g_paramCount * sizeof(GLfloat), 
		(GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Vertex color attribute buffer.
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, g_paramCount * sizeof(GLfloat), 
		(GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// Index buffer.
	glGenBuffers(1, &mesh->ib);

	mesh->vertices.reserve(262144);
	mesh->indices.reserve(524288);
}

static void DestroyMesh(Mesh* mesh)
{
	glDeleteBuffers(1, &mesh->vb);
	glDeleteBuffers(1, &mesh->ib);
	glDeleteVertexArrays(1, &mesh->va);
}

inline void SetVertex(Mesh* mesh, float x, float y, float z, float u, float v, float tex, 
	float r, float g, float b, float a)
{
	mesh->vertices.push_back(x);
	mesh->vertices.push_back(y);
	mesh->vertices.push_back(z);

	mesh->vertices.push_back(u);
	mesh->vertices.push_back(v);
	mesh->vertices.push_back(tex);

	mesh->vertices.push_back(r);
	mesh->vertices.push_back(g);
	mesh->vertices.push_back(b);
	mesh->vertices.push_back(a);

	mesh->vertices.push_back(0.0f);
	mesh->vertices.push_back(0.0f);
}

inline void SetIndices(Mesh* mesh)
{
	int offset = (int)mesh->vertices.size() / g_paramCount;

	mesh->indices.push_back(offset + 2);
	mesh->indices.push_back(offset + 1);
	mesh->indices.push_back(offset);

	mesh->indices.push_back(offset + 3);
	mesh->indices.push_back(offset + 2);
	mesh->indices.push_back(offset);
}

static void FillMeshData(Mesh* mesh)
{
	int vertexCount = (int)mesh->vertices.size();
	int indexCount = (int)mesh->indices.size();

	glBindBuffer(GL_ARRAY_BUFFER, mesh->vb);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexCount, mesh->vertices.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * indexCount, mesh->indices.data(), GL_DYNAMIC_DRAW);
}

static void DrawMesh(Mesh* mesh)
{
	glBindVertexArray(mesh->va);
	glDrawElements(GL_TRIANGLES, (int)mesh->indices.size(), GL_UNSIGNED_INT, 0);
}

static void OnWindowResize(GLFWwindow* window, int width, int height)
{
	g_windowWidth = width;
	g_windowHeight = height;
	glViewport(0, 0, width, height);
}

static Camera* NewCamera(vec3 pos)
{
	Camera* cam = Calloc(Camera);
	cam->pos = pos;
	cam->sensitivity = 0.1f;
	return cam;
}

static OrbitCamera* NewOrbitCamera(vec3 pos, float radius)
{
	OrbitCamera* cam = Calloc(OrbitCamera);
	cam->pos = pos;
	cam->radius = radius;
	cam->sensitivity = 0.25f;
	return cam;
}

static void UpdateCameraVectors(Camera* cam)
{
	vec3 forward;
	forward.x = cosf(cam->pitch) * sinf(cam->yaw);
	forward.y = sinf(cam->pitch);
	forward.z = cosf(cam->pitch) * cosf(cam->yaw);

	forward = normalize(forward);
	cam->right = normalize(cross(forward, g_worldUp));
	cam->up = normalize(cross(cam->right, forward));

	cam->forward = forward;
	cam->target = cam->pos + forward;
}

static void RotateCamera(Camera* cam, float yaw, float pitch)
{
	cam->yaw += radians(yaw);
	cam->pitch += radians(pitch);

	cam->pitch = clamp(cam->pitch, -pi<float>() / 2.0f + 0.1f, pi<float>() / 2.0f - 0.1f);
	UpdateCameraVectors(cam);
}

static void RotateCamera(OrbitCamera* cam)
{
	float yaw = radians(cam->yaw);
	float pitch = radians(clamp(cam->pitch, -89.0f, 89.0f));

	vec3 pos = cam->pos, target = cam->target;
	float radius = cam->radius;

	pos.x = target.x + radius * cosf(pitch) * sinf(yaw);
	pos.y = target.y + radius * sinf(pitch);
	pos.z = target.z + radius * cosf(pitch) * cosf(yaw);

	cam->pos = pos;
}

inline mat4 GetViewMatrix(Camera* cam)
{
	return lookAt(cam->pos, cam->target, cam->up);
}

inline mat4 GetViewMatrix(OrbitCamera* cam)
{
	return lookAt(cam->pos, cam->target, g_worldUp);
}

inline void SetCameraRadius(OrbitCamera* cam, float radius)
{
	cam->radius = clamp(radius, 2.0f, 80.0f);
}

static void LoadTexture(Texture2D* tex, char* path, bool mipMaps)
{
	int width, height, components;

	uint8_t* data = stbi_load(path, &width, &height, &components, STBI_rgb_alpha);
	Assert(data != NULL);

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	if (mipMaps) glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

static void LoadTextureArray(TextureArray* tex, char** paths, bool mipMaps)
{
	int count = sb_count(paths);
	uint8_t** dataList = (uint8_t**)malloc(count * sizeof(uint8_t*));

	int width = 0, height = 0, components;

	for (int i = 0; i < count; i++)
	{
		dataList[i] = stbi_load(paths[i], &width, &height, &components, STBI_rgb_alpha);
		Assert(dataList[i] != NULL);
	}

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, *tex);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 6, GL_RGBA8, width, height, count);

	for (int i = 0; i < count; i++)
	{
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, 
			GL_UNSIGNED_BYTE, dataList[i]);
		stbi_image_free(dataList[i]);
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (mipMaps) glGenerateMipmap(GL_TEXTURE_2D_ARRAY);

	free(dataList);
}

static void OnOpenGLMessage(GLenum src, GLenum type, GLuint id, GLenum severity, 
	GLsizei length, const GLchar* msg, const void* param)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", 
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, msg);
  	abort();
}

static GLFWwindow* InitRenderer()
{
	if (!glfwInit())
	{
		DisplayError("GLFW failed to initialize.");
		return NULL;
	}

	// Window creation.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(g_windowWidth, g_windowHeight, "Voxel Engine", NULL, NULL);

	if (window == NULL) 
	{
		DisplayError("Failed to create window.");
		return NULL;
	}

	glfwMakeContextCurrent(window);

	// Set vertical synchronization to the monitor refresh rate.
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
	{
		DisplayError("Failed to initialize GLEW.");
		return NULL;
	}

	glewExperimental = GL_TRUE;

	// Allow granular sleeping for FPS control.
	if (timeBeginPeriod(1) != TIMERR_NOERROR)
	{
		DisplayError("Failed to set sleep granularity.");
		return NULL;
	}

	glClearColor(0.0f, 0.75f, 1.0f, 1.0f);
	glViewport(0, 0, g_windowWidth, g_windowHeight);
	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	#if ASSERTIONS

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback((GLDEBUGPROC)OnOpenGLMessage, 0);

	#endif

	return window;
}

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
			DisplayError("Failed to read shader file!");
			file.close();
			delete[] inputBuffer;
			return NULL;
		}

		buffer = inputBuffer;
		inputBuffer = NULL;
	}
	else
	{
		DisplayError("Could not find the shader: ");
		DisplayError(path);
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
			
			DisplayError("Error! Shader program failed to link.");
			DisplayError(errorLog);
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
			
			DisplayError("Error! Shader failed to compile.");
			DisplayError(errorLog);
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
		DisplayError("Failed to load vertex shader from file.");
		abort();
	}

	GLuint vS = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vS, 1, &vertex, NULL);
	glCompileShader(vS);
	
	if (ShaderHasErrors(vS, VERTEX_SHADER))
	{
		DisplayError("Failed to compile the vertex shader.");
		abort();
	}

	char* frag = ShaderFromFile(fragPath);

	if (frag == NULL)
	{
		DisplayError("Failed to load fragment shader from file.");
		abort();
	}

	GLuint fS = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fS, 1, &frag, NULL);
	glCompileShader(fS);
	
	if (ShaderHasErrors(fS, FRAGMENT_SHADER))
	{
		DisplayError("Failed to compile the fragment shader.");
		abort();
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vS);
	glAttachShader(program, fS);
	glLinkProgram(program);
	
	if (ShaderHasErrors(program, SHADER_PROGRAM))
	{
		DisplayError("Failed to link the shaders into the program.");
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
	unordered_map<string, GLint>::iterator it = g_uniformMap.find(name);

	if (it == g_uniformMap.end())
		g_uniformMap[name] = glGetUniformLocation(program, name);
	
	return g_uniformMap[name];
}

inline void SetUniformSampler(GLint program, GLchar* name, GLint tex)
{
	glActiveTexture(GL_TEXTURE0 + tex);
	GLint loc = GetUniformLocation(program, name);
	glUniform1i(loc, tex);
}

inline void SetUniform(GLint program, GLchar* name, GLfloat f)
{
	GLint loc = GetUniformLocation(program, name);
	glUniform1f(loc, f);
}

inline void SetUniform(GLint program, GLchar* name, vec2 v)
{
	GLint loc = GetUniformLocation(program, name);
	glUniform2f(loc, v.x, v.y);
}

inline void SetUniform(GLint program, GLchar* name, vec3 v)
{
	GLint loc = GetUniformLocation(program, name);
	glUniform3f(loc, v.x, v.y, v.z);
}

inline void SetUniform(GLint program, GLchar* name, vec4 v)
{
	GLint loc = GetUniformLocation(program, name);
	glUniform4f(loc, v.x, v.y, v.z, v.w);
}

inline void SetUniform(GLint program, GLchar* name, mat4 m)
{
	GLint loc = GetUniformLocation(program, name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(m));
}
