// Voxel Engine
// Jason Bricco

static void InitializeMesh(Mesh* mesh)
{
	glGenVertexArrays(1, &mesh->va);
	glBindVertexArray(mesh->va);

	int params = g_renderer.paramCount;

	// Vertex position attribute buffer.
	glGenBuffers(1, &mesh->vb);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vb);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, params * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(0);

	// Texture coordinates (UVs).
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, params * sizeof(GLfloat),
		(GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Vertex color attribute buffer.
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, params * sizeof(GLfloat),
		(GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// Index buffer.
	glGenBuffers(1, &mesh->ib);

	mesh->vertices.reserve(131072);
	mesh->indices.reserve(262144);
}

static void DestroyMesh(Mesh* mesh)
{
	glDeleteBuffers(1, &mesh->vb);
	glDeleteBuffers(1, &mesh->ib);
	glDeleteVertexArrays(1, &mesh->va);

	mesh->vertices.clear();
	mesh->indices.clear();
}

inline void SetMeshVertex(Mesh* mesh, float x, float y, float z, float u, float v, float tex,
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
}

inline void SetMeshIndices(Mesh* mesh)
{
	int offset = (int)mesh->vertices.size() / g_renderer.paramCount;

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexCount, mesh->vertices.data(),
		GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * indexCount, mesh->indices.data(),
		GL_DYNAMIC_DRAW);
}

static void DrawMesh(Mesh* mesh)
{
	glBindVertexArray(mesh->va);
	glDrawElements(GL_TRIANGLES, (int)mesh->indices.size(), GL_UNSIGNED_INT, 0);
}

inline void SetGraphicVertex(float* vertices, int i, float x, float y, float u, float v)
{
	i *= 4;
	vertices[i] = x;
	vertices[i + 1] = y;
	vertices[i + 2] = u;
	vertices[i + 3] = v;
}

static Graphic* CreateGraphic(int shaderID, int texture)
{
	Graphic* graphic = Calloc(Graphic);

	glGenVertexArrays(1, &graphic->va);
	glBindVertexArray(graphic->va);

	glGenBuffers(1, &graphic->vb);
	glBindBuffer(GL_ARRAY_BUFFER, graphic->vb);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(0);

	// Texture coordinates (UVs).
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Index buffer.
	glGenBuffers(1, &graphic->ib);

	SetGraphicVertex(graphic->vertices, 0, 32.0f, 0.0f, 0.0f, 1.0f);
	SetGraphicVertex(graphic->vertices, 1, 32.0f, 32.0f, 0.0f, 0.0f);
	SetGraphicVertex(graphic->vertices, 2, 0.0f, 32.0f, 1.0f, 0.0f);
	SetGraphicVertex(graphic->vertices, 3, 0.0f, 0.0f, 1.0f, 1.0f);

	graphic->indices[0] = 2;
	graphic->indices[1] = 1;
	graphic->indices[2] = 0;

	graphic->indices[3] = 3;
	graphic->indices[4] = 2;
	graphic->indices[5] = 0;

	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16, graphic->vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, graphic->ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * 6, graphic->indices, GL_STATIC_DRAW);

	graphic->shaderID = shaderID;
	graphic->texture = texture;

	return graphic;
}

static void DrawGraphic(Graphic* graphic)
{
	int ID = graphic->shaderID;
	UseShader(g_renderer.programs[ID]);

	mat4 model = translate(mat4(), vec3(graphic->pos, 0.0f));
	SetUniform(ID, "model1", model);
	SetUniform(ID, "projection1", g_renderer.ortho);

	glBindTexture(GL_TEXTURE_2D, graphic->texture);

	glBindVertexArray(graphic->va);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

static void OnWindowResize(GLFWwindow* window, int width, int height)
{
	g_renderer.windowWidth = width;
	g_renderer.windowHeight = height;
	glViewport(0, 0, width, height);
}

static Camera* NewCamera(vec3 pos)
{
	Camera* cam = Calloc(Camera);
	cam->pos = pos;
	cam->sensitivity = 0.05f;
	return cam;
}

static void UpdateCameraVectors(Camera* cam)
{
	vec3 forward;
	forward.x = cosf(cam->pitch) * sinf(cam->yaw);
	forward.y = sinf(cam->pitch);
	forward.z = cosf(cam->pitch) * cosf(cam->yaw);

	forward = normalize(forward);
	cam->right = normalize(cross(forward, g_renderer.worldUp));
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

inline void UpdateViewMatrix()
{
	Camera* cam = g_renderer.camera;
	g_renderer.view = lookAt(cam->pos, cam->target, cam->up);
}

static void LoadTexture(GLuint* tex, char* path)
{
	int width, height, components;

	uint8_t* data = stbi_load(path, &width, &height, &components, STBI_rgb_alpha);
	Assert(data != NULL);

	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

static void LoadTextureArray(GLuint* tex, char** paths, bool mipMaps)
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
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

static void OnOpenGLMessage(GLenum src, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* msg, const void* param)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, msg);
  	abort();
}

inline int WindowWidth()
{
	return g_renderer.windowWidth;
}

inline int WindowHeight()
{
	return g_renderer.windowHeight;
}

inline void SetCamera(Camera* cam)
{
	g_renderer.camera = cam;
}

static GLFWwindow* InitRenderer()
{
	g_renderer.worldUp = vec3(0.0f, 1.0f, 0.0f);
	g_renderer.windowWidth = 1024;
	g_renderer.windowHeight = 768;
	g_renderer.paramCount = 10;

	if (!glfwInit())
	{
		LogError("GLFW failed to initialize.");
		return NULL;
	}

	// Window creation.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(WindowWidth(), WindowHeight(), "Voxel Engine", NULL, NULL);

	if (window == NULL)
	{
		LogError("Failed to create window.");
		return NULL;
	}

	glfwMakeContextCurrent(window);

	// Set vertical synchronization to the monitor refresh rate.
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
	{
		LogError("Failed to initialize GLEW.");
		return NULL;
	}

	glewExperimental = GL_TRUE;

	// Allow granular sleeping for FPS control.
	if (timeBeginPeriod(1) != TIMERR_NOERROR)
	{
		LogError("Failed to set sleep granularity.");
		return NULL;
	}

	glClearColor(0.53f, 0.80f, 0.92f, 1.0f);
	glViewport(0, 0, WindowWidth(), WindowHeight());
	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	#if ASSERTIONS

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback((GLDEBUGPROC)OnOpenGLMessage, 0);

	#endif

	g_renderer.programs[0] = LoadShader("Shaders\\diffuse_array.shader");
	g_renderer.programs[1] = LoadShader("Shaders\\crosshair.shader");

	GLuint blockTextures;

	char** paths = NULL;
	sb_push(paths, PathToAsset("Assets/Grass.png"));
	sb_push(paths, PathToAsset("Assets/GrassSide.png"));
	sb_push(paths, PathToAsset("Assets/Dirt.png"));
	sb_push(paths, PathToAsset("Assets/Crosshair.png"));

	LoadTextureArray(&blockTextures, paths, true);
	sb_free(paths);

	g_renderer.blockTextures = blockTextures;

	GLuint crosshair;
	LoadTexture(&crosshair, PathToAsset("Assets/Crosshair.png"));

	Graphic* graphic = CreateGraphic(1, crosshair);
	graphic->pos = vec2((WindowWidth() / 2.0f) - 16.0f, (WindowHeight() / 2.0f) - 16.0f);

	g_renderer.crosshair = graphic;

	g_renderer.perspective = perspective(radians(CAMERA_FOV), (float)WindowWidth() / (float)WindowHeight(),
		0.1f, 500.0f);
	g_renderer.ortho = ortho(0.0f, (float)WindowWidth(), (float)WindowHeight(), 0.0f, -1.0f, 1.0f);

	return window;
}

static Ray ScreenCenterToRay()
{
	mat4 projection = g_renderer.perspective * g_renderer.view;
	int w = WindowWidth();
	int h = WindowHeight();
	vec4 viewport = vec4(0.0f, h, w, -h);

	ivec2 cursor = ivec2(w / 2, h / 2);

	vec3 origin = unProject(vec3(cursor, 0), mat4(), projection, viewport);

	return { origin, g_renderer.camera->forward };
}

static void RenderScene(World* world)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	UpdateViewMatrix();

	UseShader(g_renderer.programs[0]);
	SetUniform(0, "view0", g_renderer.view);
	SetUniform(0, "projection0", g_renderer.perspective);
	SetUniform(0, "ambient0", vec4(1.0f, 1.0f, 1.0f, 1.0f));

	glBindTexture(GL_TEXTURE_2D_ARRAY, g_renderer.blockTextures);

	mat4 model;

	for (int i = 0; i < world->loadedCount; i++)
	{
		Chunk* next = world->loadedChunks[i];

		if (next->state == CHUNK_BUILT)
		{
			model = translate(mat4(), (vec3)next->wPos);
			SetUniform(0, "model0", model);
			DrawMesh(&next->mesh);
		}
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

	DrawGraphic(g_renderer.crosshair);

	glDisable(GL_BLEND);
}
