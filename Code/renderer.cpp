//
// Jason Bricco
//

static inline void UseShader(Shader shader)
{
    glUseProgram(shader.handle);
}

static Graphic* CreateGraphic(Shader shader, Texture texture)
{
	Graphic* graphic = new Graphic();
	graphic->mesh = CreateMesh(16, 6);

	VertexSpec spec = { true, 2, true, 2, false, 0 };

	SetMeshIndices(graphic->mesh, 4);

	SetMeshVertex(graphic->mesh, 32.0f, 0.0f, 0.0f, 1.0f);
	SetMeshVertex(graphic->mesh, 32.0f, 32.0f, 0.0f, 0.0f);
	SetMeshVertex(graphic->mesh, 0.0f, 32.0f, 1.0f, 0.0f);
	SetMeshVertex(graphic->mesh, 0.0f, 0.0f, 1.0f, 1.0f);
	
	FillMeshData(graphic->mesh, GL_STATIC_DRAW, spec);

	graphic->shader = shader;
	graphic->texture = texture;

	return graphic;
}

static void DrawGraphic(Camera* cam, Graphic* graphic)
{
	Shader shader = graphic->shader;
	UseShader(shader);
	SetUniform(shader.proj, cam->ortho);

	glBindTexture(GL_TEXTURE_2D, graphic->texture.id);
	DrawMesh(graphic->mesh, shader, graphic->pos);
}

static inline void SetCrosshairPos(Graphic* crosshair, int width, int height)
{
	crosshair->pos = vec3((width / 2.0f) - 16.0f, (height / 2.0f) - 16.0f, 0.0f);
}

static void SetWindowSize(GLFWwindow* window, int width, int height)
{
	GameState* state = (GameState*)glfwGetWindowUserPointer(window);
	Camera* cam = state->camera;

	state->windowWidth = width;
	state->windowHeight = height;
	glViewport(0, 0, width, height);

	float fov = radians(CAMERA_FOV);
	float ratio = (float)width / (float)height;

	float t = (float)tan(fov * 0.5f);
	cam->nearH = cam->nearDist * t;
	cam->nearW = cam->nearH * ratio;
	cam->farH = cam->farDist * t;
	cam->farW = cam->farH * ratio;

	cam->perspective = perspective(fov, ratio, cam->nearDist, cam->farDist);
	cam->ortho = ortho(0.0f, (float)width, (float)height, 0.0f);

	if (cam->crosshair != nullptr)
		SetCrosshairPos(cam->crosshair, width, height);
}

static Camera* NewCamera()
{
	Camera* cam = new Camera();
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

	cam->pitch = glm::clamp(cam->pitch, -PI / 2.0f + 0.1f, PI / 2.0f - 0.1f);
	UpdateCameraVectors(cam);
}

static inline void MoveCamera(Camera* cam, vec3 pos)
{	
	cam->pos = vec3(pos.x, pos.y + 1.15f, pos.z);
	UpdateCameraVectors(cam);
}

static inline void UpdateViewMatrix(Camera* cam)
{
	cam->view = lookAt(cam->pos, cam->target, cam->up);
}

#pragma warning(suppress: 4100)
static void OnOpenGLMessage(GLenum, GLenum type, GLuint, GLenum severity, GLsizei, GLchar* msg, void*)
{
	if (type != GL_DEBUG_TYPE_ERROR)
		return;

	Print("GL CALLBACK: type = 0x%x, severity = 0x%x, message = %s\n", type, severity, msg);
}

static void InitRenderer(GameState* state, Camera* cam, int screenWidth, int screenHeight)
{
	Graphic* graphic = CreateGraphic(GetShader(state, SHADER_CROSSHAIR), GetTexture(state, IMAGE_CROSSHAIR));
	SetCrosshairPos(graphic, screenWidth, screenHeight);
	
	cam->crosshair = graphic;

	for (int i = 0; i < CHUNK_MESH_COUNT; i++)
		cam->meshLists[i].reserve(512);

	cam->fadeMesh = CreateMesh(16, 4);
	VertexSpec fadeSpec = { true, 2, false, 0, false, 0 };

	SetMeshIndices(cam->fadeMesh, 4);

	SetMeshVertex(cam->fadeMesh, -1.0f, 1.0f);
	SetMeshVertex(cam->fadeMesh, 1.0f, 1.0f);
	SetMeshVertex(cam->fadeMesh, 1.0f, -1.0f);
	SetMeshVertex(cam->fadeMesh, -1.0f, -1.0f);
	
	FillMeshData(cam->fadeMesh, GL_STATIC_DRAW, fadeSpec);

	cam->fadeShader = GetShader(state, SHADER_FADE);
	cam->fadeColor = CLEAR_COLOR;
}

static Ray ScreenCenterToRay(GameState* state, Camera* cam)
{
	mat4 projection = cam->perspective * cam->view;

	int w = state->windowWidth;
	int h = state->windowHeight;
	vec4 viewport = vec4(0.0f, (float)h, (float)w, (float)(-h));

	ivec2 cursor = ivec2(w / 2, h / 2);

	vec3 origin = unProject(vec3(cursor, 0.0f), mat4(1.0f), projection, viewport);

	return { origin, cam->forward };
}

// The six camera frustum planes can be obtained using the eight points that define the corners
// of the view frustum. This is an optimized version.
static inline void GetCameraPlanes(Camera* cam)
{	
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

static void RenderScene(GameState* state, Camera* cam)
{
	BEGIN_TIMED_BLOCK(RENDER_SCENE);

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	UpdateViewMatrix(cam);

	// Opaque pass.
	Shader shader = GetShader(state, SHADER_DIFFUSE_ARRAY);

	UseShader(shader);
	SetUniform(shader.view, cam->view);
	SetUniform(shader.proj, cam->perspective);
	SetUniform(shader.ambient, state->ambient);

	glBindTexture(GL_TEXTURE_2D_ARRAY, GetTextureArray(state, IMAGE_ARRAY_BLOCKS).id);
	int count = (int)cam->meshLists[MESH_TYPE_OPAQUE].size();

	for (int i = 0; i < count; i++)
	{
		ChunkMesh cM = cam->meshLists[MESH_TYPE_OPAQUE][i];
		DrawMesh(cM.mesh, shader, cM.pos);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Fluid pass.
	shader = GetShader(state, SHADER_FLUID_ARRAY);

	UseShader(shader);
	SetUniform(shader.view, cam->view);
	SetUniform(shader.proj, cam->perspective);
	SetUniform(shader.time, cam->animTime);
	SetUniform(shader.ambient, state->ambient);

	if (cam->disableFluidCull) 
		glDisable(GL_CULL_FACE);

	count = (int)cam->meshLists[MESH_TYPE_FLUID].size();

	for (int i = 0; i < count; i++)
	{
		ChunkMesh cM = cam->meshLists[MESH_TYPE_FLUID][i];
		DrawMesh(cM.mesh, shader, cM.pos);
	}

	if (cam->disableFluidCull) 
		glEnable(GL_CULL_FACE);

	glDisable(GL_DEPTH_TEST);

	if (cam->fadeColor != CLEAR_COLOR)
	{
		shader = cam->fadeShader;

		UseShader(shader);
		SetUniform(shader.color, cam->fadeColor);
		DrawMesh(cam->fadeMesh);
	}

	if (!g_paused)
	{
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO);
		DrawGraphic(cam, cam->crosshair);
	}

	glDisable(GL_BLEND);

	glClear(GL_DEPTH_BUFFER_BIT);

	END_TIMED_BLOCK(RENDER_SCENE);
}

static Texture LoadTexture(int width, int height, uint8_t* pixels)
{
	Texture tex;

    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

static Texture LoadTextureArray(TextureArrayData& data, char* assetData)
{
	Texture tex;

	glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex.id);

    ImageData first = data[0];
    int width = first.width, height = first.height;
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 6, GL_RGBA8, width, height, (GLsizei)data.size());

	for (int i = 0; i < data.size(); i++)
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, assetData + data[i].pixels);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    return tex;
}

static void OutputShaderError(GLuint shader, char* mode)
{
	GLint length = 0;
	glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);

    GLchar* errorLog = (GLchar*)calloc(length, sizeof(GLchar));
    glGetProgramInfoLog(shader, length, NULL, errorLog);
    
   	Print("Error! Shader program failed to %s. Log: %s\n", mode, length == 0 ? "No error given." : errorLog);
   	
    free(errorLog);
    Unused(mode);
}

static bool ShaderHasErrors(GLuint shader, ShaderType type)
{
    int status = 0;

    if (type == SHADER_PROGRAM)
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &status);

        if (status == GL_FALSE)
        {
        	OutputShaderError(shader, "link");
            return true;
        }
    }
    else
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        if (status == GL_FALSE)
        {
        	OutputShaderError(shader, "compile");
            return true;
        }
    }

    return false;
}

static Shader LoadShader(int vertLength, char* vertCode, int fragLength, char* fragCode)
{
    GLuint vS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vS, 1, &vertCode, &vertLength);
    glCompileShader(vS);
    
    if (ShaderHasErrors(vS, VERTEX_SHADER))
       	Error("Failed to compile the vertex shader.\n");

    GLuint fS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fS, 1, &fragCode, &fragLength);
    glCompileShader(fS);
    
    if (ShaderHasErrors(fS, FRAGMENT_SHADER))
        Error("Failed to compile the fragment shader.\n");

    GLuint program = glCreateProgram();
    glAttachShader(program, vS);
    glAttachShader(program, fS);
    glLinkProgram(program);
    
    if (ShaderHasErrors(program, SHADER_PROGRAM))
        Error("Failed to link the shaders into the program.\n");

    glDeleteShader(vS);
    glDeleteShader(fS);

    return { program };
}
