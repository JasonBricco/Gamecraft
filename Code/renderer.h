// Voxel Engine
// Jason Bricco

#define CAMERA_FOV 45.0f

struct Mesh
{
	GLuint vb, ib, va;

	vector<float> vertices;
	vector<int> indices;
};

struct Graphic
{
	GLuint vb, ib, va;
	int indices[6];
	float vertices[16];
	int shaderID;
	GLuint texture;
	vec2 pos;
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

struct Renderer
{
	GLFWwindow* window;
	Camera* camera;

	int windowWidth, windowHeight;

	vec3 worldUp;

	unordered_map<string, GLint> uniforms;
	int paramCount;

	mat4 perspective, ortho;
	mat4 view;

	Graphic* crosshair;

	// Textures.
	GLuint blockTextures;

	GLuint programs[2];

	ivec3 cursor;
};
