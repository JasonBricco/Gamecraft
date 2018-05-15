// Voxel Engine
// Jason Bricco

#define MESH_PARAMS 10
#define CAMERA_FOV 45.0f
#define WORLD_UP vec3(0.0f, 1.0f, 0.0f)

struct Graphic
{
	GLuint vb, ib, va;
	int indices[6];
	float vertices[16];
	int shaderID;
	GLuint texture;
	vec2 pos;
};

enum FrustumVisibility
{
	FRUSTUM_INVISIBLE,
	FRUSTUM_VISIBLE,
	FRUSTUM_PARTIAL
};

// Define a plane as having a point, p, and a normal, n.
struct Plane
{
	vec3 p, n;
};

struct Camera
{
	vec3 pos, target, up;
	vec3 forward, right;

	// Distance to the near and far frustum planes.
	float nearDist, farDist;

	// Width and height values for the near and far frustum planes.
	float nearW, farW;
	float nearH, farH;

	// Rotation angles in degrees.
	float yaw, pitch;

	float sensitivity;

	Plane planes[6];
};

struct Renderer
{
	Camera* camera;

	int windowWidth, windowHeight;

	unordered_map<string, GLint> uniforms;

	mat4 perspective, ortho, view;

	Graphic* crosshair;

	// Textures.
	GLuint blockTextures;

	GLuint programs[2];
};
