// Voxel Engine
// Jason Bricco

#define MESH_PARAMS 10
#define CAMERA_FOV 45.0f
#define WORLD_UP NewV3(0.0f, 1.0f, 0.0f)

struct Graphic
{
	GLuint vb, ib, va;
	int indices[6];
	float vertices[16];
	int shaderID;
	GLuint texture;
	Vec2 pos;
};

// Standard first-person camera.
struct Camera
{
	Vec3 pos, target, up;
	Vec3 forward, right;

	// Angles in degrees.
	float yaw, pitch;

	float sensitivity;
};

struct Renderer
{
	Camera* camera;

	int windowWidth, windowHeight;

	unordered_map<string, GLint> uniforms;

	Matrix4 perspective, ortho;
	Matrix4 view;

	Graphic* crosshair;

	// Textures.
	GLuint blockTextures;

	GLuint programs[2];
};

inline void SetGraphicVertex(float* vertices, int i, float x, float y, float u, float v);
static Graphic* CreateGraphic(int shaderID, int texture);
static void DrawGraphic(Renderer* rend, Graphic* graphic);

inline void SetCrosshairPos(Graphic* crosshair, int width, int height);

static void SetWindowSize(GLFWwindow* window, int width, int height);

static Camera* NewCamera();
static void UpdateCameraVectors(Camera* cam);
static void RotateCamera(Camera* cam, float yaw, float pitch);
inline void UpdateViewMatrix(Renderer* rend);

static void LoadTexture(GLuint* tex, char* path);
static void LoadTextureArray(GLuint* tex, char** paths, bool mipMaps);

static void OnOpenGLMessage(GLenum src, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* msg, const void* param);

static GLFWwindow* InitRenderer(Renderer* rend);

static Ray ScreenCenterToRay(Renderer* rend);

static void RenderScene(Renderer* rend, World* world);
