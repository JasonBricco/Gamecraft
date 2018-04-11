// Voxel Engine
// Jason Bricco

#define Texture2D GLuint
#define TextureArray GLuint

#define CAMERA_FOV 45.0f

enum ShaderType
{
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	SHADER_PROGRAM
};

struct Mesh
{
	GLuint vb, ib, va;

	vector<float> vertices;
	vector<int> indices;
};

// Standard first-person camera.
struct Camera
{
	vec3 pos, target, up;
	vec3 forward, right;

	// Angles in degrees.
	float yaw, pitch;

	float sensitivity;
};

// Camera that orbits around a center point, 'pos'.
struct OrbitCamera
{
	vec3 pos, target;
	float yaw, pitch;
	float radius;
	float sensitivity;
};

struct Renderer
{
	Camera* cam;
	OrbitCamera* orbit;
};

static void InitializeMesh(Mesh* mesh);
static void DestroyMesh(Mesh* mesh);
static void DrawMesh(Mesh* mesh);

inline void SetVertex(Mesh* mesh, float x, float y, float z, float u, float v, float tex, 
	float r, float g, float b, float a);
inline void SetIndices(Mesh* mesh);

// Transfers mesh data on the CPU to the GPU.
static void FillMeshData(Mesh* mesh);

static void OnWindowResize(GLFWwindow* window, int width, int height);

static Camera* NewCamera(vec3 pos);
static OrbitCamera* NewOrbitCamera(vec3 pos, float radius);
static void UpdateCameraVectors(Camera* cam);
static void RotateCamera(Camera* cam, float yaw, float pitch);
static void RotateCamera(OrbitCamera* cam);
inline void SetCameraRadius(OrbitCamera* cam, float radius);

inline mat4 GetViewMatrix(Camera* cam);
inline mat4 GetViewMatrix(OrbitCamera* cam);

static void LoadTexture(Texture2D* tex, char* path, bool mipMaps = true);
static void LoadTextureArray(TextureArray* tex, char** paths, bool mipMaps = true);

// Called when OpenGL encounters an error. Replaces need for glGetError() calls.
static void OnOpenGLMessage(GLenum src, GLenum type, GLuint id, GLenum severity, 
	GLsizei length, const GLchar* msg, const void* param);

static GLFWwindow* InitRenderer();

static char* ShaderFromFile(char* fileName);
static bool ShaderHasErrors(GLuint handle, ShaderType type);
static GLuint LoadShaders(char* vertexPath, char* fragPath);
inline void UseShader(GLuint program);

static GLint GetUniformLocation(GLint program, GLchar* name);
inline void SetUniformSampler(GLint program, GLchar* name, GLint tex);
inline void SetUniform(GLint program, GLchar* name, GLfloat f);
inline void SetUniform(GLint program, GLchar* name, vec2 v);
inline void SetUniform(GLint program, GLchar* name, vec3 v);
inline void SetUniform(GLint program, GLchar* name, vec4 v);
inline void SetUniform(GLint program, GLchar* name, mat4 m);
