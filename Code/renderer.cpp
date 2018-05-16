// Voxel Engine
// Jason Bricco

inline void SetGraphicVertex(float* vertices, int i, float x, float y, float u, float v)
{
	i *= 4;
	vertices[i] = x;
	vertices[i + 1] = y;
	vertices[i + 2] = u;
	vertices[i + 3] = v;
}

static Graphic* CreateGraphic(Renderer* rend, int shaderID, int texture)
{
	Graphic* graphic = Calloc(Graphic, sizeof(Graphic), "Graphic");

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

	graphic->u_model = glGetUniformLocation(rend->programs[shaderID], "model");
	graphic->u_proj = glGetUniformLocation(rend->programs[shaderID], "projection");

	return graphic;
}

static void DrawGraphic(Renderer* rend, Graphic* graphic)
{
	int ID = graphic->shaderID;
	UseShader(rend->programs[ID]);

	mat4 model = translate(mat4(1.0f), vec3(graphic->pos, 0.0f));
	SetUniform(graphic->u_model, model);
	SetUniform(graphic->u_proj, rend->ortho);

	glBindTexture(GL_TEXTURE_2D, graphic->texture);

	glBindVertexArray(graphic->va);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

inline void SetCrosshairPos(Graphic* crosshair, int width, int height)
{
	crosshair->pos = vec2((width / 2.0f) - 16.0f, (height / 2.0f) - 16.0f);
}

static void SetWindowSize(GLFWwindow* window, int width, int height)
{
	Renderer* rend = (Renderer*)glfwGetWindowUserPointer(window);
	rend->windowWidth = width;
	rend->windowHeight = height;
	glViewport(0, 0, width, height);

	Camera* cam = rend->camera;

	float fov = radians(CAMERA_FOV);
	float ratio = (float)width / (float)height;

	float t = (float)tan(fov * 0.5f);
	cam->nearH = cam->nearDist * t;
	cam->nearW = cam->nearH * ratio;
	cam->farH = cam->farDist * t;
	cam->farW = cam->farH * ratio;

	rend->perspective = perspective(fov, ratio, cam->nearDist, cam->farDist);
	rend->ortho = ortho(0.0f, (float)width, (float)height, 0.0f);

	if (rend->crosshair != NULL)
		SetCrosshairPos(rend->crosshair, width, height);
}

static Camera* NewCamera()
{
	Camera* cam = Calloc(Camera, sizeof(Camera), "Cam");
	cam->nearDist = 0.1f;
	cam->farDist = 512.0f;
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
	cam->right = normalize(cross(forward, WORLD_UP));
	cam->up = normalize(cross(cam->right, forward));

	cam->forward = forward;
	cam->target = cam->pos + forward;
}

static void RotateCamera(Camera* cam, float yaw, float pitch)
{
	cam->yaw += radians(yaw);
	cam->pitch += radians(pitch);

	cam->pitch = clamp(cam->pitch, -PI / 2.0f + 0.1f, PI / 2.0f - 0.1f);
	UpdateCameraVectors(cam);
}

inline void CameraFollow(Player* player)
{	
	vec3 pos = player->pos;
	player->camera->pos = vec3(pos.x, pos.y + 1.15f, pos.z);
	UpdateCameraVectors(player->camera);
}

inline void UpdateViewMatrix(Renderer* rend)
{
	Camera* cam = rend->camera;
	rend->view = lookAt(cam->pos, cam->target, cam->up);
}

static void LoadTexture(GLuint* tex, char* asset)
{
	int width, height, components;

	char* path = PathToAsset(asset);
	uint8_t* data = stbi_load(path, &width, &height, &components, STBI_rgb_alpha);
	Free(path, "AssetPath");
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
	uint8_t** dataList = Malloc(uint8_t*, count * sizeof(uint8_t*), "DataList");

	int width = 0, height = 0, components;

	for (int i = 0; i < count; i++)
	{
		char* path = PathToAsset(paths[i]);
		dataList[i] = stbi_load(path, &width, &height, &components, STBI_rgb_alpha);
		Free(path, "AssetPath");
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

	Free(dataList, "DataList");
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

static void OnOpenGLMessage(GLenum src, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* msg, const void* param)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, msg);
  	abort();
}

static GLFWwindow* InitRenderer(Renderer* rend)
{
	if (!glfwInit())
	{
		OutputDebugString("GLFW failed to initialize.");
		return NULL;
	}

	rend->camera = NewCamera();

	// Window creation.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	int screenWidth = 1024, screenHeight = 768;

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Voxel Engine", NULL, NULL);

	if (window == NULL)
	{
		OutputDebugString("Failed to create window.");
		return NULL;
	}

	glfwSetWindowUserPointer(window, rend);
	SetWindowSize(window, screenWidth, screenHeight);
	glfwMakeContextCurrent(window);

	// Set vertical synchronization to the monitor refresh rate.
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
	{
		OutputDebugString("Failed to initialize GLEW.");
		return NULL;
	}

	glewExperimental = GL_TRUE;

	glClearColor(0.53f, 0.80f, 0.92f, 1.0f);
	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	#if ASSERTIONS

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback((GLDEBUGPROC)OnOpenGLMessage, 0);

	#endif

	rend->programs[0] = LoadShader("Shaders\\diffuse_array.shader");
	rend->programs[1] = LoadShader("Shaders\\Crosshair.shader");

	rend->u_view = glGetUniformLocation(rend->programs[0], "view");
	rend->u_model = glGetUniformLocation(rend->programs[0], "model");
	rend->u_proj = glGetUniformLocation(rend->programs[0], "projection");

	GLuint blockTextures;

	char** paths = NULL;

	sb_push(paths, "Assets/Grass.png");
	sb_push(paths, "Assets/GrassSide.png");
	sb_push(paths, "Assets/Dirt.png");
	sb_push(paths, "Assets/Stone.png");
	sb_push(paths, "Assets/Water.png");
	sb_push(paths, "Assets/Sand.png");
	sb_push(paths, "Assets/Crate.png");
	sb_push(paths, "Assets/StoneBrick.png");

	LoadTextureArray(&blockTextures, paths, true);
	sb_free(paths);

	rend->blockTextures = blockTextures;

	GLuint Crosshair;
	LoadTexture(&Crosshair, "Assets/Crosshair.png");

	Graphic* graphic = CreateGraphic(rend, 1, Crosshair);
	SetCrosshairPos(graphic, screenWidth, screenHeight);
	
	rend->crosshair = graphic;

	return window;
}

static Ray ScreenCenterToRay(Renderer* rend)
{
	mat4 projection = rend->perspective * rend->view;

	int w = rend->windowWidth;
	int h = rend->windowHeight;
	vec4 viewport = vec4(0.0f, (float)h, (float)w, (float)(-h));

	ivec2 cursor = ivec2(w / 2, h / 2);

	vec3 origin = unProject(vec3(cursor, 0.0f), mat4(1.0f), projection, viewport);

	return { origin, rend->camera->forward };
}

// The six camera frustum planes can be obtained using the eight points the define the corners
// of the view frustum. This is an optimized version.
inline void GetCameraPlanes(Camera* cam)
{	
	BEGIN_TIMED_BLOCK(CAMERA_PLANES);

	vec3 fc = cam->pos + cam->forward * cam->farDist;
	vec3 nc = cam->pos + cam->forward * cam->nearDist;

	// Near plane.
	cam->planes[0] = { nc, cam->forward };

	// Far plane.
	cam->planes[1] = { fc, -cam->forward };

	// Top plane.
	vec3 aux = normalize((nc + cam->up * cam->nearH) - cam->pos);
	vec3 n = cross(aux, cam->right);
	cam->planes[2] = { nc + cam->up * cam->nearH, n };

	// Bottom plane.
	aux = normalize((nc - cam->up * cam->nearH) - cam->pos);
	n = cross(cam->right, aux);
	cam->planes[3] = { nc - cam->up * cam->nearH, n };

	// Left plane.
	aux = normalize((nc - cam->right * cam->nearW) - cam->pos);
	n = cross(aux, cam->up);
	cam->planes[4] = { nc - cam->right * cam->nearW, n };

	// Right plane.
	aux = normalize((nc + cam->right * cam->nearW) - cam->pos);
	n = cross(cam->up, aux);
	cam->planes[5] = { nc + cam->right * cam->nearW, n };

	END_TIMED_BLOCK(CAMERA_PLANES);
}

// Returns the farthest positive vertex from an AABB defined by min and max
// along the given normal.
inline vec3 FarthestPositiveVertex(vec3 min, vec3 max, vec3 normal)
{
	vec3 v = min;

	if (normal.x >= 0.0f)
		v.x = max.x;

	if (normal.y >= 0.0f)
		v.y = max.y;

	if (normal.z >= 0.0f)
		v.z = max.z;

	return v;
}

inline vec3 FarthestNegativeVertex(vec3 min, vec3 max, vec3 normal)
{
	vec3 v = max;

	if (normal.x >= 0.0f)
		v.x = min.x;

	if (normal.y >= 0.0f)
		v.y = min.y;

	if (normal.z >= 0.0f)
		v.z = min.z;

	return v;
}

inline float SignedDist(Plane plane, vec3 v) 
{
    return dot(plane.n, (v - plane.p));
}

inline FrustumVisibility TestFrustum(Camera* cam, vec3 min, vec3 max)
{
	for (int i = 0; i < 6; i++)
	{
		Plane plane = cam->planes[i];

		vec3 vert = FarthestPositiveVertex(min, max, plane.n);
		float dist = SignedDist(plane, vert);

		if (dist < 0.0f) 
			return FRUSTUM_INVISIBLE;

		vert = FarthestNegativeVertex(min, max, plane.n);
		dist = SignedDist(plane, vert);

		if (dist < 0.0f) 
			return FRUSTUM_PARTIAL;
	}

	return FRUSTUM_VISIBLE;
}

static void RenderScene(Renderer* rend, World* world)
{
	BEGIN_TIMED_BLOCK(RENDER_SCENE);

	glClear(GL_COLOR_BUFFER_BIT);

	UpdateViewMatrix(rend);

	UseShader(rend->programs[0]);
	SetUniform(rend->u_view, rend->view);
	SetUniform(rend->u_proj, rend->perspective);

	glBindTexture(GL_TEXTURE_2D_ARRAY, rend->blockTextures);

	mat4 model;

	for (int i = 0; i < world->visibleCount; i++)
	{
		Chunk* chunk = world->visibleChunks[i];

		if (chunk->state >= CHUNK_BUILT && chunk->mesh->vertCount > 0)
		{
			model = translate(mat4(1.0f), (vec3)chunk->lwPos);
			SetUniform(rend->u_model, model);
			DrawMesh(chunk->mesh);
		}
	}

	if (!g_paused)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);

		DrawGraphic(rend, rend->crosshair);

		glDisable(GL_BLEND);
	}

	glClear(GL_DEPTH_BUFFER_BIT);

	END_TIMED_BLOCK(RENDER_SCENE);
}
