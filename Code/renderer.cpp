//
// Jason Bricco
//

static inline void UseShader(Shader shader)
{
    glUseProgram(shader.handle);
}

static Graphic* CreateGraphic(Renderer* rend, Shader shader, Texture texture)
{
	Graphic* graphic = Calloc<Graphic>();
	CreateMesh2D(&graphic->mesh, 16, 6);

	SetMeshIndices2D(&graphic->mesh);

	SetMeshVertex2D(&graphic->mesh, 32.0f, 0.0f, 0.0f, 1.0f);
	SetMeshVertex2D(&graphic->mesh, 32.0f, 32.0f, 0.0f, 0.0f);
	SetMeshVertex2D(&graphic->mesh, 0.0f, 32.0f, 1.0f, 0.0f);
	SetMeshVertex2D(&graphic->mesh, 0.0f, 0.0f, 1.0f, 1.0f);
	
	FillMeshData2D(&graphic->mesh);

	graphic->shader = shader;
	graphic->texture = texture;

	return graphic;
}

static void DrawGraphic(Renderer* rend, Graphic* graphic)
{
	Shader shader = graphic->shader;
	UseShader(shader);
	SetUniform(shader.proj, rend->ortho);

	glBindTexture(GL_TEXTURE_2D, graphic->texture);
	DrawMesh2D(&graphic->mesh, shader, graphic->pos);
}

static inline void SetCrosshairPos(Graphic* crosshair, int width, int height)
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

	if (rend->crosshair != nullptr)
		SetCrosshairPos(rend->crosshair, width, height);
}

static Camera* NewCamera()
{
	Camera* cam = Calloc<Camera>();
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

static inline void CameraFollow(Player* player)
{	
	vec3 pos = player->pos;
	player->camera->pos = vec3(pos.x, pos.y + 1.15f, pos.z);
	UpdateCameraVectors(player->camera);
}

static inline void UpdateViewMatrix(Renderer* rend)
{
	Camera* cam = rend->camera;
	rend->view = lookAt(cam->pos, cam->target, cam->up);
}

static void OnOpenGLMessage(GLenum src, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* msg, const void* param)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, msg);
}

static void InitRenderer(Renderer* rend, Assets* assets, int screenWidth, int screenHeight)
{
	rend->camera = NewCamera();

	Graphic* graphic = CreateGraphic(rend, assets->crosshair, assets->crosshairTex);
	SetCrosshairPos(graphic, screenWidth, screenHeight);
	
	rend->crosshair = graphic;

	for (int i = 0; i < CHUNK_MESH_COUNT; i++)
		rend->meshLists[i].reserve(512);
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
static inline void GetCameraPlanes(Camera* cam)
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
static inline vec3 FarthestPositiveVertex(vec3 min, vec3 max, vec3 normal)
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

static inline vec3 FarthestNegativeVertex(vec3 min, vec3 max, vec3 normal)
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

static inline float SignedDist(Plane plane, vec3 v) 
{
    return dot(plane.n, (v - plane.p));
}

static inline FrustumVisibility TestFrustum(Camera* cam, vec3 min, vec3 max)
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

static void RenderScene(Renderer* rend, Assets* assets)
{
	BEGIN_TIMED_BLOCK(RENDER_SCENE);

	glClear(GL_COLOR_BUFFER_BIT);

	UpdateViewMatrix(rend);

	// Opaque pass.
	Shader shader = assets->diffuseArray;

	UseShader(shader);
	SetUniform(shader.view, rend->view);
	SetUniform(shader.proj, rend->perspective);

	glBindTexture(GL_TEXTURE_2D_ARRAY, assets->blockTextures);
	int count = (int)rend->meshLists[MESH_TYPE_OPAQUE].size();

	for (int i = 0; i < count; i++)
		DrawMesh(rend->meshLists[MESH_TYPE_OPAQUE][i], shader);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Fluid pass.
	shader = assets->fluidArray;

	UseShader(shader);
	SetUniform(shader.view, rend->view);
	SetUniform(shader.proj, rend->perspective);
	SetUniform(shader.time, rend->animTime);

	count = (int)rend->meshLists[MESH_TYPE_FLUID].size();

	for (int i = 0; i < count; i++)
		DrawMesh(rend->meshLists[MESH_TYPE_FLUID][i], shader);

	if (!g_paused)
	{
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
		DrawGraphic(rend, rend->crosshair);
	}

	glDisable(GL_BLEND);
	glClear(GL_DEPTH_BUFFER_BIT);

	END_TIMED_BLOCK(RENDER_SCENE);
}
