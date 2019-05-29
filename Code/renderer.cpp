//
// Gamecraft
//

static inline void SetUniform(GLint loc, GLfloat f)
{
    glUniform1f(loc, f);
}

static inline void SetUniform(GLint loc, GLint f)
{
    glUniform1i(loc, f);
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

static Graphic* CreateGraphic(Renderer& renderer, Shader* shader, Texture texture)
{
	Graphic* graphic = new Graphic();

	MeshData2D* data = GetMeshData(renderer.meshData2D);
	assert(data != nullptr);

	SetIndices(data);
	SetUVs(data);

	data->positions[0] = vec3(32.0f, 0.0f, 0.0f);
	data->positions[1] = vec3(32.0f, 32.0f, 0.0f);
	data->positions[2] = vec3(0.0f, 32.0f, 0.0f);
	data->positions[3] = vec3(0.0f, 0.0f, 0.0f);
	
	FillMeshData(renderer.meshData2D, graphic->mesh, data, GL_STATIC_DRAW, MESH_NO_COLORS);

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

static void DestroyAAFBO(Renderer& rend)
{
	GLuint textures[] = { rend.colAA, rend.depthAA };
	glDeleteTextures(2, textures);
	glDeleteFramebuffers(1, &rend.fboAA);
}

static void CreateAAFBO(Renderer& rend)
{
	if (rend.samplesAA > 0)
	{
		if (glIsBuffer(rend.fboAA))
			DestroyAAFBO(rend);

		ivec2 size = FramebufferSize();

		glGenTextures(1, &rend.colAA);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rend.colAA);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, rend.samplesAA, GL_RGBA8, size.x, size.y, true);

		glGenTextures(1, &rend.depthAA);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, rend.depthAA);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, rend.samplesAA, GL_DEPTH_COMPONENT24, size.x, size.y, true);

		glGenFramebuffers(1, &rend.fboAA);
		glBindFramebuffer(GL_FRAMEBUFFER, rend.fboAA);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, rend.colAA, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, rend.depthAA, 0);

		CheckFrameBufferStatus();
	}
}

static void SetAA(Renderer& rend, int samples)
{
	rend.samplesAA = samples;

	if (samples > 0)
	{
		glEnable(GL_MULTISAMPLE);
		CreateAAFBO(rend);
	}
	else
	{
		glDisable(GL_MULTISAMPLE);
		DestroyAAFBO(rend);
	}
}

static void SetWindowSize(GLFWwindow* window, int width, int height)
{
	GameState* state = (GameState*)glfwGetWindowUserPointer(window);

	if (width == 0 || height == 0)
	{
		state->minimized = true;
		return;
	}

	state->minimized = false;

	Camera* cam = state->camera;

	glViewport(0, 0, width, height);

	float fov = radians(CAMERA_FOV);
	float ratio = (float)width / (float)height;

	float t = (float)tan(fov * 0.5f);
	cam->nearH = cam->nearDist * t;
	cam->nearW = cam->nearH * ratio;
	cam->farH = cam->farDist * t;
	cam->farW = cam->farH * ratio;

	Renderer& rend = state->renderer;
	rend.perspective = perspective(fov, ratio, cam->nearDist, cam->farDist);

	if (rend.crosshair != nullptr)
		SetCrosshairPos(rend.crosshair, width, height);

	CreateAAFBO(rend);
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
}

static inline void MoveCamera(Camera* cam, vec3 pos)
{	
	cam->pos = vec3(pos.x, pos.y + 0.45f, pos.z);
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

static void ApplyClearColor(GameState* state, Renderer& rend)
{
	glClearColor(rend.clearColor.r, rend.clearColor.g, rend.clearColor.b, 1.0f);

	AssetDatabase& db = state->assets;

	Shader* opaque = &db.shaders[SHADER_BLOCK_OPAQUE];
	UseShader(opaque);
	SetUniform(opaque->fogColor, rend.clearColor);

	Shader* opaqueAnim = &db.shaders[SHADER_BLOCK_OPAQUE_ANIMATED];
	UseShader(opaqueAnim);
	SetUniform(opaqueAnim->fogColor, rend.clearColor);

	Shader* alpha = &db.shaders[SHADER_BLOCK_TRANSPARENT];
	UseShader(alpha);
    SetUniform(alpha->fogColor, rend.clearColor);

	Shader* alphaAnim = &db.shaders[SHADER_BLOCK_TRANSPARENT_ANIMATED];
	UseShader(alphaAnim);
    SetUniform(alphaAnim->fogColor, rend.clearColor);
}

static void InitRenderer(GameState* state, Renderer& rend, int screenWidth, int screenHeight)
{
	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_CULL_FACE);

	#if _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback((GLDEBUGPROC)OnOpenGLMessage, 0);
	#endif

	AssetDatabase& db = state->assets;
	float fogStart = 100.0f, fogEnd = 208.0f;

    Shader* opaque = &db.shaders[SHADER_BLOCK_OPAQUE];
    opaque->view = glGetUniformLocation(opaque->handle, "view");
    opaque->model = glGetUniformLocation(opaque->handle, "model");
    opaque->proj = glGetUniformLocation(opaque->handle, "projection");
    opaque->ambient = glGetUniformLocation(opaque->handle, "ambient");
    opaque->fogStart = glGetUniformLocation(opaque->handle, "fogStart");
    opaque->fogEnd = glGetUniformLocation(opaque->handle, "fogEnd");
    opaque->fogColor = glGetUniformLocation(opaque->handle, "fogColor");

    UseShader(opaque);
    SetUniform(opaque->fogStart, fogStart);
    SetUniform(opaque->fogEnd, fogEnd);

    Shader* opaqueAnim = &db.shaders[SHADER_BLOCK_OPAQUE_ANIMATED];
    opaqueAnim->view = glGetUniformLocation(opaqueAnim->handle, "view");
    opaqueAnim->model = glGetUniformLocation(opaqueAnim->handle, "model");
    opaqueAnim->proj = glGetUniformLocation(opaqueAnim->handle, "projection");
    opaqueAnim->ambient = glGetUniformLocation(opaqueAnim->handle, "ambient");
    opaqueAnim->fogStart = glGetUniformLocation(opaqueAnim->handle, "fogStart");
    opaqueAnim->fogEnd = glGetUniformLocation(opaqueAnim->handle, "fogEnd");
    opaqueAnim->fogColor = glGetUniformLocation(opaqueAnim->handle, "fogColor");
    opaqueAnim->animIndex = glGetUniformLocation(opaqueAnim->handle, "animIndex");

    UseShader(opaqueAnim);
    SetUniform(opaqueAnim->fogStart, fogStart);
    SetUniform(opaqueAnim->fogEnd, fogEnd);

    Shader* transparent = &db.shaders[SHADER_BLOCK_TRANSPARENT];
    transparent->view = glGetUniformLocation(transparent->handle, "view");
    transparent->model = glGetUniformLocation(transparent->handle, "model");
    transparent->proj = glGetUniformLocation(transparent->handle, "projection");
    transparent->ambient = glGetUniformLocation(transparent->handle, "ambient");
    transparent->fogStart = glGetUniformLocation(transparent->handle, "fogStart");
    transparent->fogEnd = glGetUniformLocation(transparent->handle, "fogEnd");
    transparent->fogColor = glGetUniformLocation(transparent->handle, "fogColor");
    transparent->animIndex = glGetUniformLocation(transparent->handle, "animIndex");

    UseShader(transparent);
    SetUniform(transparent->fogStart, fogStart);
    SetUniform(transparent->fogEnd, fogEnd);

    Shader* transparentAnim = &db.shaders[SHADER_BLOCK_TRANSPARENT_ANIMATED];
    transparentAnim->view = glGetUniformLocation(transparentAnim->handle, "view");
    transparentAnim->model = glGetUniformLocation(transparentAnim->handle, "model");
    transparentAnim->proj = glGetUniformLocation(transparentAnim->handle, "projection");
    transparentAnim->ambient = glGetUniformLocation(transparentAnim->handle, "ambient");
    transparentAnim->fogStart = glGetUniformLocation(transparentAnim->handle, "fogStart");
    transparentAnim->fogEnd = glGetUniformLocation(transparentAnim->handle, "fogEnd");
    transparentAnim->fogColor = glGetUniformLocation(transparentAnim->handle, "fogColor");
    transparentAnim->animIndex = glGetUniformLocation(transparentAnim->handle, "animIndex");

    UseShader(transparentAnim);
    SetUniform(transparentAnim->fogStart, fogStart);
    SetUniform(transparentAnim->fogEnd, fogEnd);

    Shader* crosshair = &db.shaders[SHADER_CROSSHAIR];
    crosshair->model = glGetUniformLocation(crosshair->handle, "model");
    crosshair->proj = glGetUniformLocation(crosshair->handle, "projection");

    Shader* ui = &db.shaders[SHADER_UI];
    ui->proj = glGetUniformLocation(ui->handle, "proj");

    Shader* fade = &db.shaders[SHADER_FADE];
    fade->fadeColor = glGetUniformLocation(fade->handle, "inColor");
    rend.fadeShader = fade;

    Shader* particle = &db.shaders[SHADER_PARTICLE];
    particle->view = glGetUniformLocation(particle->handle, "view");
    particle->proj = glGetUniformLocation(particle->handle, "projection");

	Graphic* graphic = CreateGraphic(rend, GetShader(state, SHADER_CROSSHAIR), GetTexture(state, IMAGE_CROSSHAIR));
	SetCrosshairPos(graphic, screenWidth, screenHeight);
	
	rend.crosshair = graphic;

	MeshData2D* data = GetMeshData(rend.meshData2D);
	assert(data != nullptr);

	SetIndices(data);

	data->positions[0] = vec3(-1.0f, 1.0f, 0.0f);
	data->positions[1] = vec3(1.0f, 1.0f, 0.0f);
	data->positions[2] = vec3(1.0f, -1.0f, 0.0f);
	data->positions[3] = vec3(-1.0f, -1.0f, 0.0f);
	
	FillMeshData(rend.meshData2D, rend.fadeMesh, data, GL_STATIC_DRAW, MESH_NO_COLORS | MESH_NO_UVS);

	rend.fadeColor = CLEAR_COLOR;
}

static Ray ScreenCenterToRay(GameState* state, Camera* cam)
{
	Renderer& rend = state->renderer;
	mat4 projection = rend.perspective * cam->view;

	ivec2 size = FramebufferSize();
	vec4 viewport = vec4(0.0f, (float)size.y, (float)size.x, (float)(-size.y));

	ivec2 cursor = ivec2(size.x / 2, size.y / 2);

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
	TIMED_FUNCTION;
	
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

static void RenderScene(GameState* state, Renderer& rend, Camera* cam)
{
	TIMED_FUNCTION;

	if (rend.samplesAA > 0)
		glBindFramebuffer(GL_FRAMEBUFFER, rend.fboAA);
	else glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	UpdateViewMatrix(cam);

	glBindTexture(GL_TEXTURE_2D_ARRAY, GetBlockTextureArray(state).id);

	// Opaque pass.
	Shader* shader = GetShader(state, SHADER_BLOCK_OPAQUE);

	UseShader(shader);
	SetUniform(shader->view, cam->view);
	SetUniform(shader->proj, rend.perspective);
	SetUniform(shader->ambient, rend.ambient);

	size_t count = rend.meshLists[MESH_OPAQUE].size();

	for (int i = 0; i < count; i++)
	{
		ChunkMesh cM = rend.meshLists[MESH_OPAQUE][i];
		DrawMesh(cM.mesh, shader, cM.pos, MESH_OPAQUE);
	}

	// Opaque animated pass.
	shader = GetShader(state, SHADER_BLOCK_OPAQUE_ANIMATED);

	int animIndex = (int)PingPong(state->time * 2.0f, 4.0f);

	UseShader(shader);
	SetUniform(shader->view, cam->view);
	SetUniform(shader->proj, rend.perspective);
	SetUniform(shader->ambient, rend.ambient);
	SetUniform(shader->animIndex, animIndex);

	count = rend.meshLists[MESH_OPAQUE_ANIMATED].size();

	for (int i = 0; i < count; i++)
	{
		ChunkMesh cM = rend.meshLists[MESH_OPAQUE_ANIMATED][i];
		DrawMesh(cM.mesh, shader, cM.pos, MESH_OPAQUE_ANIMATED);
	}

	// Transparent pass.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader = GetShader(state, SHADER_BLOCK_TRANSPARENT);

	UseShader(shader);
	SetUniform(shader->view, cam->view);
	SetUniform(shader->proj, rend.perspective);
	SetUniform(shader->ambient, rend.ambient);

	count = rend.meshLists[MESH_TRANSPARENT].size();

	for (int i = 0; i < count; i++)
	{
		ChunkMesh cM = rend.meshLists[MESH_TRANSPARENT][i];
		DrawMesh(cM.mesh, shader, cM.pos, MESH_TRANSPARENT);
	}

	// Transparent animated pass.
	shader = GetShader(state, SHADER_BLOCK_TRANSPARENT_ANIMATED);

	animIndex = (int)PingPong(state->time * 2.0f, 7.0f);

	UseShader(shader);
	SetUniform(shader->view, cam->view);
	SetUniform(shader->proj, rend.perspective);
	SetUniform(shader->animIndex, animIndex);
	SetUniform(shader->ambient, rend.ambient);

	if (rend.disableFluidCull) 
		glDisable(GL_CULL_FACE);

	count = rend.meshLists[MESH_TRANSPARENT_ANIMATED].size();

	for (int i = 0; i < count; i++)
	{
		ChunkMesh cM = rend.meshLists[MESH_TRANSPARENT_ANIMATED][i];
		DrawMesh(cM.mesh, shader, cM.pos, MESH_TRANSPARENT_ANIMATED);
	}

	// If we didn't already disable culling, disable it now for particle drawing.
	if (!rend.disableFluidCull)
		glDisable(GL_CULL_FACE);

	for (int i = 0; i < rend.emitters.size(); i++)
		DrawParticles(state, *rend.emitters[i], cam);

	rend.emitters.clear();

	glDisable(GL_DEPTH_TEST);

	if (rend.fadeColor != CLEAR_COLOR)
	{
		shader = rend.fadeShader;
		UseShader(shader);
		SetUniform(shader->fadeColor, rend.fadeColor);
		DrawMesh(rend.fadeMesh);
	}

	RenderUI(state, rend, state->ui);

	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (rend.samplesAA > 0)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, rend.fboAA);
		glDrawBuffer(GL_BACK);

		ivec2 size = FramebufferSize();
		glBlitFramebuffer(0, 0, size.x, size.y, 0, 0, size.x, size.y, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}
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

    return tex;
}

static void OutputShaderError(GLuint shader, char* mode)
{
	GLint length = 0;
	glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &length);

    GLchar* errorLog = new GLchar[length];
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
