//
// Jason Bricco
//

static inline void SetUniform(GLint loc, GLfloat f)
{
    glUniform1f(loc, f);
}

static inline void SetUniform(GLint loc, vec2 v)
{
    glUniform2f(loc, v.x, v.y);
}

static inline void SetUniform(GLint loc, vec3 v)
{
    glUniform3f(loc, v.x, v.y, v.z);
}

static inline void SetUniform(GLint loc, vec4 v)
{
    glUniform4f(loc, v.x, v.y, v.z, v.w);
}

static inline void SetUniform(GLint loc, mat4 m)
{
    glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(m));
}

static inline void UseShader(Shader* shader)
{
    glUseProgram(shader->handle);
}

static Graphic* CreateGraphic(Shader* shader, Texture texture)
{
	Graphic* graphic = CallocStruct(Graphic);

	MeshData* data = CreateMeshData(4, 6);

	SetIndices(data);
	SetUVs(data, 0);

	data->positions[0] = vec3(32.0f, 0.0f, 0.0f);
	data->positions[1] = vec3(32.0f, 32.0f, 0.0f);
	data->positions[2] = vec3(0.0f, 32.0f, 0.0f);
	data->positions[3] = vec3(0.0f, 0.0f, 0.0f);
	data->vertCount = 4;
	
	FillMeshData(graphic->mesh, data, GL_STATIC_DRAW, MESH_NO_COLORS);

	graphic->shader = shader;
	graphic->texture = texture;

	return graphic;
}

static void DrawGraphic(Graphic* graphic, Shader* shader)
{
	glBindTexture(GL_TEXTURE_2D, graphic->texture.id);
	DrawMesh(graphic->mesh, shader, graphic->pos);
}

static inline void SetCrosshairPos(Graphic* crosshair, int width, int height)
{
	crosshair->pos = vec3((width / 2.0f) - 16.0f, (height / 2.0f) - 16.0f, 0.0f);
}

static void CheckFrameBufferStatus()
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		switch (status)
		{
			case GL_FRAMEBUFFER_UNDEFINED:
				Error("Frame buffer is not complete. Status: Framebuffer Undefined\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				Error("Frame buffer is not complete. Status: Incomplete Attachment\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				Error("Frame buffer is not complete. Status: Incomplete Missing Attachment\n");
				break;

			case GL_FRAMEBUFFER_UNSUPPORTED:
				Error("Frame buffer is not complete. Status: Framebuffer Unsupported\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				Error("Frame buffer is not complete. Status: Incomplete Draw Buffer\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				Error("Frame buffer is not complete. Status: Incomplete Read Buffer\n");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
				Error("Frame buffer is not complete. Status: Incomplete Multisample\n");
				break;
			
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				Error("Frame buffer is not complete. Status: Incomplete Layer Targets\n");
				break;

			default:
				Error("Frame buffer is not complete. Status: Unknown, Code: %i\n", status);
		}
	}
}

static void DestroyAAFBO(Camera* cam)
{
	GLuint textures[] = { cam->colAA, cam->depthAA };
	glDeleteTextures(2, textures);
	glDeleteFramebuffers(1, &cam->fboAA);
	UntrackGLAllocs(3);
}

static void CreateAAFBO(Camera* cam, int width, int height)
{
	if (cam->samplesAA > 0)
	{
		if (glIsBuffer(cam->fboAA))
			DestroyAAFBO(cam);

		glGenTextures(1, &cam->colAA);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, cam->colAA);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cam->samplesAA, GL_RGBA8, width, height, true);

		glGenTextures(1, &cam->depthAA);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, cam->depthAA);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, cam->samplesAA, GL_DEPTH_COMPONENT24, width, height, true);

		glGenFramebuffers(1, &cam->fboAA);
		glBindFramebuffer(GL_FRAMEBUFFER, cam->fboAA);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, cam->colAA, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, cam->depthAA, 0);

		CheckFrameBufferStatus();
		TrackGLAllocs(3);
	}
}

static void SetAA(GameState* state, int samples)
{
	Camera* cam = state->camera;
	cam->samplesAA = samples;

	if (samples > 0)
	{
		glEnable(GL_MULTISAMPLE);
		CreateAAFBO(cam, state->windowWidth, state->windowHeight);
	}
	else
	{
		glDisable(GL_MULTISAMPLE);
		DestroyAAFBO(cam);
	}
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

	if (cam->crosshair != nullptr)
		SetCrosshairPos(cam->crosshair, width, height);

	CreateAAFBO(cam, width, height);
}

static Camera* NewCamera()
{
	Camera* cam = CallocStruct(Camera);
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
}

static inline void MoveCamera(Camera* cam, vec3 pos)
{	
	cam->pos = vec3(pos.x, pos.y + 1.15f, pos.z);
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
	DebugBreak();
}

static void ListUniforms(Shader* shader)
{
	GLint count;
	glGetProgramiv(shader->handle, GL_ACTIVE_UNIFORMS, &count);

	for (int i = 0; i < count; i++) 
	{
	    char name[100];
	    glGetActiveUniformName(shader->handle, i, sizeof(name), (GLsizei*)NULL, name);
	   	Print("%s\n", name);
	}
}

static void InitRenderer(GameState* state, Camera* cam, int screenWidth, int screenHeight)
{
	state->ambient = 1.0f;
	vec3 clearColor = vec3(0.53f, 0.80f, 0.92f);
	state->clearColor = clearColor;

	glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_CULL_FACE);

#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback((GLDEBUGPROC)OnOpenGLMessage, 0);
#endif

	AssetDatabase& db = state->assets;
	float fogStart = 100.0f, fogEnd = 208.0f;

    Shader* diffuseArray = &db.shaders[SHADER_DIFFUSE_ARRAY];
    diffuseArray->view = glGetUniformLocation(diffuseArray->handle, "view");
    diffuseArray->model = glGetUniformLocation(diffuseArray->handle, "model");
    diffuseArray->proj = glGetUniformLocation(diffuseArray->handle, "projection");
    diffuseArray->ambient = glGetUniformLocation(diffuseArray->handle, "ambient");
    diffuseArray->fogStart = glGetUniformLocation(diffuseArray->handle, "fogStart");
    diffuseArray->fogEnd = glGetUniformLocation(diffuseArray->handle, "fogEnd");
    diffuseArray->fogColor = glGetUniformLocation(diffuseArray->handle, "fogColor");

    UseShader(diffuseArray);
    SetUniform(diffuseArray->fogStart, fogStart);
    SetUniform(diffuseArray->fogEnd, fogEnd);
    SetUniform(diffuseArray->fogColor, clearColor);

    Shader* fluidArray = &db.shaders[SHADER_FLUID_ARRAY];
    fluidArray->view = glGetUniformLocation(fluidArray->handle, "view");
    fluidArray->model = glGetUniformLocation(fluidArray->handle, "model");
    fluidArray->proj = glGetUniformLocation(fluidArray->handle, "projection");
    fluidArray->time = glGetUniformLocation(fluidArray->handle, "time");
    fluidArray->ambient = glGetUniformLocation(fluidArray->handle, "ambient");
    fluidArray->fogStart = glGetUniformLocation(fluidArray->handle, "fogStart");
    fluidArray->fogEnd = glGetUniformLocation(fluidArray->handle, "fogEnd");
    fluidArray->fogColor = glGetUniformLocation(fluidArray->handle, "fogColor");

    UseShader(fluidArray);
    SetUniform(fluidArray->fogStart, fogStart);
    SetUniform(fluidArray->fogEnd, fogEnd);
    SetUniform(fluidArray->fogColor, clearColor);

    Shader* crosshair = &db.shaders[SHADER_CROSSHAIR];
    crosshair->model = glGetUniformLocation(crosshair->handle, "model");
    crosshair->proj = glGetUniformLocation(crosshair->handle, "projection");

    Shader* ui = &db.shaders[SHADER_UI];
    ui->proj = glGetUniformLocation(ui->handle, "proj");

    Shader* fade = &db.shaders[SHADER_FADE];
    fade->fadeColor = glGetUniformLocation(fade->handle, "inColor");

    Shader* particle = &db.shaders[SHADER_PARTICLE];
    particle->view = glGetUniformLocation(particle->handle, "view");
    particle->proj = glGetUniformLocation(particle->handle, "projection");

	Graphic* graphic = CreateGraphic(GetShader(state, SHADER_CROSSHAIR), GetTexture(state, IMAGE_CROSSHAIR));
	SetCrosshairPos(graphic, screenWidth, screenHeight);
	
	cam->crosshair = graphic;

	MeshData* data = CreateMeshData(4, 6);
	SetIndices(data);

	data->positions[0] = vec3(-1.0f, 1.0f, 0.0f);
	data->positions[1] = vec3(1.0f, 1.0f, 0.0f);
	data->positions[2] = vec3(1.0f, -1.0f, 0.0f);
	data->positions[3] = vec3(-1.0f, -1.0f, 0.0f);
	data->vertCount = 4;
	
	FillMeshData(cam->fadeMesh, data, GL_STATIC_DRAW, MESH_NO_UVS | MESH_NO_COLORS);

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
	if (cam->samplesAA > 0)
		glBindFramebuffer(GL_FRAMEBUFFER, cam->fboAA);
	else glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	UpdateViewMatrix(cam);

	// Opaque pass.
	Shader* shader = GetShader(state, SHADER_DIFFUSE_ARRAY);

	UseShader(shader);
	SetUniform(shader->view, cam->view);
	SetUniform(shader->proj, cam->perspective);
	SetUniform(shader->ambient, state->ambient);

	glBindTexture(GL_TEXTURE_2D_ARRAY, GetTextureArray(state, IMAGE_ARRAY_BLOCKS).id);
	int count = cam->meshLists[MESH_TYPE_OPAQUE].count;

	for (int i = 0; i < count; i++)
	{
		ChunkMesh cM = cam->meshLists[MESH_TYPE_OPAQUE].meshes[i];
		DrawMesh(cM.mesh, shader, cM.pos);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Fluid pass.
	shader = GetShader(state, SHADER_FLUID_ARRAY);

	UseShader(shader);
	SetUniform(shader->view, cam->view);
	SetUniform(shader->proj, cam->perspective);
	assert(cam->animTime >= 0.0f && cam->animTime < 1.0f);
	SetUniform(shader->time, cam->animTime);
	SetUniform(shader->ambient, state->ambient);

	if (cam->disableFluidCull) 
		glDisable(GL_CULL_FACE);

	count = cam->meshLists[MESH_TYPE_FLUID].count;

	for (int i = 0; i < count; i++)
	{
		ChunkMesh cM = cam->meshLists[MESH_TYPE_FLUID].meshes[i];
		DrawMesh(cM.mesh, shader, cM.pos);
	}

	// If we didn't already disable culling, disable it now for particle drawing.
	if (!cam->disableFluidCull)
		glDisable(GL_CULL_FACE);

	DrawParticles(state, state->rain, cam);

	glDisable(GL_DEPTH_TEST);

	if (cam->fadeColor != CLEAR_COLOR)
	{
		shader = cam->fadeShader;
		UseShader(shader);
		SetUniform(shader->fadeColor, cam->fadeColor);
		DrawMesh(cam->fadeMesh);
	}

	RenderUI(state, cam, state->ui);

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (cam->samplesAA > 0)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, cam->fboAA);
		glDrawBuffer(GL_BACK);
		glBlitFramebuffer(0, 0, state->windowWidth, state->windowHeight, 0, 0, state->windowWidth, state->windowHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}
}

static Texture LoadTexture(int width, int height, uint8_t* pixels)
{
	Texture tex;

    glGenTextures(1, &tex.id);
    TrackGLAllocs(1);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

static Texture LoadTextureArray(ImageData* data, int count, char* assetData)
{
	Texture tex;

	glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex.id);

    ImageData first = data[0];
    int width = first.width, height = first.height;
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 6, GL_RGBA8, width, height, count);

	for (int i = 0; i < count; i++)
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, assetData + data[i].pixels);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    TrackGLAllocs(2);

    return tex;
}

static void OutputShaderError(GLuint shader, char* mode)
{
	GLint length = 0;
	glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);

    GLchar* errorLog = CallocArray(length, GLchar);
    glGetProgramInfoLog(shader, length, NULL, errorLog);
    
   	Print("Error! Shader program failed to %s. Log: %s\n", mode, length == 0 ? "No error given." : errorLog);
   	
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

static void LoadShader(Shader* shader, int vertLength, char* vertCode, int fragLength, char* fragCode)
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

    shader->handle = program;
}
